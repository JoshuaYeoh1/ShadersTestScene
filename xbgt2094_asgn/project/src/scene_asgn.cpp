// Include this header to use any gl* functions
#include <glad/glad.h>
#include "scene_asgn.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "framework/framework.h"
#include "renderable_entity.h"
#include "lighting/light_debug.h"
#include <vector>
#include <algorithm>

#ifdef XBGT2094_ENABLE_IMGUI
#include "imgui/imgui.h"
#endif


//SKYBOX--------------------------------------------------------------------------------

static Mesh* mesh_skybox;
static Shader* shader_skybox;
static Cubemap* cubemap_skybox;

static void RenderSkybox(CameraBase* camera)
{
	glDepthMask(GL_FALSE); // disable WRITING to depth buffer. Depth test STILL OCCURS.
	glDepthFunc(GL_LEQUAL);

	SimpleRenderer::bindShader(shader_skybox);

	SimpleRenderer::setShaderProp_Mat4("projection", camera->getProjectionMatrix());
	SimpleRenderer::setShaderProp_Mat4("view", glm::mat4(glm::mat3(camera->getViewMatrix())));

	SimpleRenderer::setTexture_skybox(cubemap_skybox);
	SimpleRenderer::drawMesh(mesh_skybox);
	SimpleRenderer::setTexture_skybox(0);

	glDepthMask(GL_TRUE); // enable WRITING to depth buffer. Successful Depth test writes the new value to depth buffer.
	glDepthFunc(GL_LESS);
}

// preload() runs before loadShaders()
// Shader* variables are NOT initialized yet, referencing will not work!

void Scene_ASGN::preload()
{
	// Set to false to disable rendering grid and axes lines
	renderDebug = false;

	// Skybox loaded in preload() to declutter load()
	mesh_skybox = MeshUtils::makeSkybox();
	ShaderUtils::loadShader(&shader_skybox, "SKYBOX", "../assets/shaders/skybox.vert", "../assets/shaders/skybox.frag");
	cubemap_skybox = TextureUtils::loadCubemap("../assets/textures/skybox2/forest", "jpg");
}


//TEXTURE CONFIGS--------------------------------------------------------------------------------

static TextureConfig cfgRepeat(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureFilterMode::LINEAR, true);
static TextureConfig cfgClamp(TextureWrapMode::CLAMP, TextureWrapMode::CLAMP, TextureFilterMode::LINEAR, true);
static TextureConfig cfgRepeatPixel(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureFilterMode::NEAREST, true);
static TextureConfig cfgClampPixel(TextureWrapMode::CLAMP, TextureWrapMode::CLAMP, TextureFilterMode::NEAREST, true);


//FBO--------------------------------------------------------------------------------

static ColourDepthFBO* fbo;
static Shader* shader_screen;

static void FBOShader()	
{
	ShaderUtils::loadShader(&shader_screen, "shader_screen", "../assets/shaders/screen.vert", "../assets/shaders/screen.frag");

	SimpleRenderer::bindShader(shader_screen);
	SimpleRenderer::setShaderProp_Integer("mainTex", 0);
	SimpleRenderer::setShaderProp_Integer("scanline", 1);
}

Texture2D* scanlineTex = nullptr;

static void LoadFBO()
{
	ColourDepthFrameBufferConfig fbocfg;

	fbocfg.size = App::getViewportSize();
	fbocfg.depthFormat = DepthFormat::FLOAT24;
	fbocfg.colourAttachments.push_back(ColourAttachmentData(ColourFormat::RGBA, TextureFilterMode::LINEAR));

	fbo = FBOUtils::createColourDepthFBO(fbocfg);

	scanlineTex = TextureUtils::loadTexture2D("../assets/textures/postprocess/scanline.png", cfgRepeat);
}

static void BindFBO()
{
	SimpleRenderer::bindFBO(fbo);

	// clear the custom fbo before drawing scene
	// otherwise drawing may fail because depth test fails
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


static bool EnablePostProcess = false;

static bool EnableSpherize = true;
static float SpherizeStrength = 2;
static float ZoomScale = 0.75;

static bool EnableGrayscale = true;

static bool EnableScanlines = true;
static int ScanlineTiling = 2;
static float ScanlineScroll = 0.1;

static bool EnableVignette = true;
static float VignetteStrength = 15;
static glm::vec3 VignetteColor = { 0,0,0 };

static void RenderFBO()
{
	SimpleRenderer::bindFBO_Default();
	
	// draw a quad covering the entire screen
	static Mesh* fullscreenQuad = MeshUtils::makeQuad(2);

	SimpleRenderer::bindShader(shader_screen);

	// set shader properties
	SimpleRenderer::setShaderProp_Float("time", App::getTime());

	SimpleRenderer::setShaderProp_Bool("EnablePostProcess", EnablePostProcess);

	SimpleRenderer::setShaderProp_Bool("EnableSpherize", EnableSpherize);
	SimpleRenderer::setShaderProp_Float("SpherizeStrength", SpherizeStrength);
	SimpleRenderer::setShaderProp_Float("ZoomScale", ZoomScale);

	SimpleRenderer::setShaderProp_Bool("EnableGrayscale", EnableGrayscale);

	SimpleRenderer::setShaderProp_Bool("EnableScanlines", EnableScanlines);
	SimpleRenderer::setShaderProp_Integer("ScanlineTiling", ScanlineTiling);
	SimpleRenderer::setShaderProp_Float("ScanlineScroll", ScanlineScroll);

	SimpleRenderer::setShaderProp_Bool("EnableVignette", EnableVignette);
	SimpleRenderer::setShaderProp_Float("VignetteStrength", VignetteStrength);
	SimpleRenderer::setShaderProp_Vec3("VignetteColor", VignetteColor);

	// bind fbo colour texture to texture unit 0
	SimpleRenderer::setTexture_0(fbo->getColorAttachment(0));
	SimpleRenderer::setTexture_1(scanlineTex);

	// spawn quad
	SimpleRenderer::drawMesh(fullscreenQuad);
}



//SHADOWS--------------------------------------------------------------------------------

static unsigned int shadowMapFBO;

static glm::uvec2 SHADOW_RES = { 2048,2048 };

static unsigned int shadowMap;

static void CreateShadowMap()
{
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RES.x, SHADOW_RES.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float clampColor[] = { 1,1,1,1 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


static float shadow_bounds = 35;
static float shadow_near_plane = 1;
static float shadow_far_plane = 100;

static glm::mat4 GetLightSpaceMatrix(glm::vec3 lightDir)
{
	float mid_plane = (shadow_far_plane + shadow_near_plane) / 2;

	glm::mat4 orthoProjection = glm::ortho
	(
		-shadow_bounds, shadow_bounds, -shadow_bounds, shadow_bounds,
		shadow_near_plane, shadow_far_plane
	);

	glm::vec3 lightPos = -lightDir * mid_plane;
	glm::vec3 center = { 0,0,0 };
	glm::vec3 up = { 0,1,0 };

	glm::mat4 lightView = glm::lookAt(lightPos, center, up);

	glm::mat4 lightSpaceMatrix = orthoProjection * lightView;

	return lightSpaceMatrix;
}



//SHADERS--------------------------------------------------------------------------------
// loadShaders() run AFTER preload()
// It is also called when reload shader key is pressed (default F1)

static std::vector<DirectionalLight*> lights_directional;
static std::vector<PointLight*> lights_point;
static std::vector<SpotLight*> lights_spot;

static Shader* shader_lit;

static void StandardLitShader()
{
	ShaderUtils::loadShader(&shader_lit, "shader_lit", "../assets/shaders/standard.vert", "../assets/shaders/lit.frag");

	SimpleRenderer::bindShader(shader_lit);
	SimpleRenderer::setShaderProp_Integer("DiffuseTexture", 0);
	SimpleRenderer::setShaderProp_Integer("SpecularTexture", 1);
	SimpleRenderer::setShaderProp_Integer("NormalTexture", 2);
	SimpleRenderer::setShaderProp_Integer("EmissiveTexture", 3);
	SimpleRenderer::setShaderProp_Integer("AOTexture", 4);
	SimpleRenderer::setShaderProp_Integer("shadowMap", 5);
}

static Shader* shader_fire;

static void FireShader()
{
	ShaderUtils::loadShader(&shader_fire, "shader_fire", "../assets/shaders/fire.vert", "../assets/shaders/unlit.frag");

	SimpleRenderer::bindShader(shader_fire);
	SimpleRenderer::setShaderProp_Integer("DiffuseTexture", 0);
}

static Shader* shader_shadow;

static void ShadowShader()
{
	ShaderUtils::loadShader(&shader_shadow, "shader_shadow", "../assets/shaders/shadow.vert", "../assets/shaders/shadow.frag");
}

void Scene_ASGN::loadShaders()
{	
	StandardLitShader();
	FireShader();
	ShadowShader();
	FBOShader();
}



//RENDER LIGHTS--------------------------------------------------------------------------------

static bool EnableShadow = false;
static float ShadowStrength = 1;
static float ShadowBias = 0.0005;

static void RenderDirectionalLights()
{
	SimpleRenderer::bindShader(shader_lit);

	int num_lights = lights_directional.size();

	SimpleRenderer::setShaderProp_Integer("NUM_DIRECTIONAL_LIGHTS", num_lights);

	for (int i = 0; i < num_lights; i++)
	{
		auto light = lights_directional[i];

		// check light active
		glm::vec3 lightCol = light->getActive() ? light->getColorIntensified() : glm::vec3(0, 0, 0);

		// set light properties
		SimpleRenderer::setShaderProp_Vec3("DirectionalLights[" + std::to_string(i) + "].col", lightCol);
		SimpleRenderer::setShaderProp_Vec3("DirectionalLights[" + std::to_string(i) + "].dir", light->getDirection());

		// set shadow properties
		SimpleRenderer::setShaderProp_Bool("EnableShadow", EnableShadow);
		SimpleRenderer::setShaderProp_Float("ShadowStrength", ShadowStrength);
		SimpleRenderer::setShaderProp_Float("ShadowBias", ShadowBias);

		// set shadowMap
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		SimpleRenderer::setShaderProp_Integer("shadowMap", 5);

		// Calculate and set the light space matrix
		glm::vec3 lightDir = lights_directional[i]->getDirection();
		glm::mat4 lightSpaceMatrix = GetLightSpaceMatrix(lightDir);
		SimpleRenderer::setShaderProp_Mat4("lightProjection", lightSpaceMatrix);
	}	
}

static void RenderPointLights()
{
	SimpleRenderer::bindShader(shader_lit);

	int num_lights = lights_point.size();

	SimpleRenderer::setShaderProp_Integer("NUM_POINT_LIGHTS", num_lights);

	for (int i = 0; i < num_lights; i++)
	{
		auto light = lights_point[i];

		// check light active
		glm::vec3 lightCol = light->getActive() ? light->getColorIntensified() : glm::vec3(0, 0, 0);

		// set light properties
		SimpleRenderer::setShaderProp_Vec3("PointLights[" + std::to_string(i) + "].col", lightCol);
		SimpleRenderer::setShaderProp_Float("PointLights[" + std::to_string(i) + "].range", light->getInverseSquaredRange());
		SimpleRenderer::setShaderProp_Vec3("PointLights[" + std::to_string(i) + "].pos", light->getPosition());

		// Calculate and set the light space matrix
		//glm::mat4 lightSpaceMatrix = GetLightSpaceMatrixPoint(light->getPosition());
		//SimpleRenderer::setShaderProp_Mat4("PointLights[" + std::to_string(i) + "].lightSpaceMatrix", lightSpaceMatrix);

		// Set the shadow map texture
		//int texID = light->getShadowMapTextureID();
		//SimpleRenderer::setTexture_X(light->getDepthFBO(), texID);
	}
}

static void RenderSpotLights()
{
	SimpleRenderer::bindShader(shader_lit);

	int num_lights = lights_spot.size();

	SimpleRenderer::setShaderProp_Integer("NUM_SPOT_LIGHTS", num_lights);

	for (int i = 0; i < num_lights; i++)
	{
		auto light = lights_spot[i];

		// check light active
		glm::vec3 lightCol = light->getActive() ? light->getColorIntensified() : glm::vec3(0, 0, 0);

		// set light properties
		SimpleRenderer::setShaderProp_Vec3("SpotLights[" + std::to_string(i) + "].col", lightCol);
		SimpleRenderer::setShaderProp_Vec3("SpotLights[" + std::to_string(i) + "].dir", light->getDirection());
		SimpleRenderer::setShaderProp_Vec3("SpotLights[" + std::to_string(i) + "].pos", light->getPosition());
		SimpleRenderer::setShaderProp_Float("SpotLights[" + std::to_string(i) + "].range", light->getInverseSquaredRange());
		SimpleRenderer::setShaderProp_Vec2("SpotLights[" + std::to_string(i) + "].angles", light->getCalculatedAngles());

		// Calculate and set the light space matrix
		//glm::mat4 lightSpaceMatrix = GetLightSpaceMatrixSpot(light);
		//SimpleRenderer::setShaderProp_Mat4("SpotLights[" + std::to_string(i) + "].lightSpaceMatrix", lightSpaceMatrix);

		// Set the shadow map texture
		//int texID = light->getShadowMapTextureID();
		//SimpleRenderer::setTexture_X(light->getDepthFBO(), texID);
	}
}



//RENDER OBJECTS--------------------------------------------------------------------------------

static bool enableDiffuse = true;
static bool enableSpecular = true;
static bool enableNormal = true;
static bool enableEmissive = true;
static bool enableAO = true;

static void RenderObject(RenderableEntity& entity, CameraBase* camera)
{
	if (entity.doubleSided) glDisable(GL_CULL_FACE);

	// 1. Bind the shader for this entity
	SimpleRenderer::bindShader(entity.shader);

	// 2. Set shader properties
	SimpleRenderer::setShaderProp_Mat4("projection", camera->getProjectionMatrix());
	SimpleRenderer::setShaderProp_Mat4("view", camera->getViewMatrix());
	SimpleRenderer::setShaderProp_Mat4("model", entity.getModelMatrix());
	SimpleRenderer::setShaderProp_Float("time", App::getTime());
	SimpleRenderer::setShaderProp_Float("Shininess", entity.shininess);
	SimpleRenderer::setShaderProp_Float("AlphaClip", entity.alphaClip);
	SimpleRenderer::setShaderProp_Float("BreathingSpeed", entity.breathingSpeed);
	SimpleRenderer::setShaderProp_Vec3("Tint", entity.tint);
	SimpleRenderer::setShaderProp_Float("Opacity", entity.opacity);

	// 3. Set material properties of this entity

	Texture2D* diffuseTex = enableDiffuse ? entity.diffuseTex : TextureUtils::whiteTexture2D();
	Texture2D* specularTex = enableSpecular ? entity.specularTex : TextureUtils::whiteTexture2D();
	Texture2D* normalTex = enableNormal ? entity.normalTex : TextureUtils::whiteTexture2D();
	Texture2D* emissiveTex = enableEmissive ? entity.emissiveTex : TextureUtils::blackTexture2D();
	Texture2D* aoTex = enableAO ? entity.aoTex : TextureUtils::whiteTexture2D();

	SimpleRenderer::setTexture_0(diffuseTex);
	SimpleRenderer::setTexture_1(specularTex);
	SimpleRenderer::setTexture_2(normalTex);
	SimpleRenderer::setTexture_3(emissiveTex);
	SimpleRenderer::setTexture_4(aoTex);

	// 4. draw the mesh of this entity
	SimpleRenderer::drawMesh(entity.mesh);

	glEnable(GL_CULL_FACE);
}

static std::vector<RenderableEntity*> entities_lit;

static void RenderLitObjects(CameraBase* camera)
{
	// Iterate through all opaque entities
	for(auto it : entities_lit)
	{
		auto& entity = *it; // Alias *it as entity for readability purposes

		if(!entity.active) continue;

		RenderObject(entity, camera);
	}
}

static std::vector<RenderableEntity*> entities_alphablend;

static void RenderAlphaBlends(CameraBase* camera)
{
	auto entities_alphablend_copy = entities_alphablend;

	std::sort
	(
		entities_alphablend_copy.begin(),
		entities_alphablend_copy.end(),

		[&camera](const RenderableEntity* el1, const RenderableEntity* el2)
		{
			// calc distance of el1 to the camera
			float el1_dist = glm::length2(el1->getPosition() - camera->getPosition());
			// calc distance of el2 to the camera
			float el2_dist = glm::length2(el2->getPosition() - camera->getPosition());

			return el1_dist > el2_dist;
		}
	);

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Iterate through all alpha-blended entities
	for(auto it : entities_alphablend_copy)
	{
		auto& entity = *it; // Alias *it as entity for readability purposes

		if (!entity.active) continue;

		RenderObject(entity, camera);
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

static std::vector<RenderableEntity*> pivots;



//PARENT LIGHTS--------------------------------------------------------------------------------

static void ParentLight(RenderableEntity* parent, PointLight* light)
{
	light->setParentModelMatrix(parent->getModelMatrix());
}

static void ParentLight(RenderableEntity* parent, SpotLight* light)
{
	light->setParentModelMatrix(parent->getModelMatrix());
}

static void UpdateLightsParenting()
{
	ParentLight(entities_alphablend[0], lights_point[0]); // blue gem light
	ParentLight(entities_alphablend[1], lights_point[1]); // green gem light
	ParentLight(entities_alphablend[2], lights_point[2]); // purple gem light
	ParentLight(entities_alphablend[3], lights_point[3]); // red gem light
	ParentLight(entities_alphablend[4], lights_point[4]); // fire light 1
	ParentLight(entities_alphablend[5], lights_point[5]); // fire light 2
	ParentLight(entities_alphablend[6], lights_point[6]); // fire light 3
	ParentLight(entities_alphablend[7], lights_point[7]); // fire light 4

	// could use forloop, but the arrays being parallel is just a coincidence
}



//SPAWN--------------------------------------------------------------------------------

static void SpawnSunlight()
{
	DirectionalLight* light = LightUtils::createDirectionalLight("Sunlight");

	light->setColour({ 1, 1, 1 });
	light->setIntensity(0.5);
	light->setDirection({ -1, -2, -1 });

	//light->loadDepthFBO();

	lights_directional.push_back(light);
}

static void SpawnBase()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Base";
	entity->mesh = MeshUtils::loadObjFile("../assets/models/base/base.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgRepeat;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/base/base.jpg", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/base/base_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/base/base_n.jpg", cfg);
	//entity->emissiveTex = 
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/base/base_ao.jpg", cfg);

	//entity->shininess = 0;
	//entity->alphaClip = 0.1;
	//entity->doubleSided = false;

	entity->position = glm::vec3(0.1, -1.7, 2.7);
	entity->rotation = glm::vec3(0, 0, 0);
	entity->scale = glm::vec3(2);

	//entity->parent = 

	//entity->active = true;

	entities_lit.push_back(entity);
}

static void SpawnVine1()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Vine1";
	entity->mesh = MeshUtils::loadObjFile("../assets/models/base/vine1.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgClamp;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/base/vine1.png", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/base/vine1_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/base/vine1_n.jpg", cfg);
	//entity->emissiveTex = 
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/base/vine1_ao.jpg", cfg);

	//entity->shininess = 0;
	entity->alphaClip = 0.9;
	entity->doubleSided = true;

	entity->position = glm::vec3(0, 0, 0);
	entity->rotation = glm::vec3(0, 0, 0);
	entity->scale = glm::vec3(1);

	entity->parent = entities_lit[0]; // base

	entities_lit.push_back(entity);
}

static void SpawnVine2()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Vine2";
	entity->mesh = MeshUtils::loadObjFile("../assets/models/base/vine2.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgClamp;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/base/vine2.png", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/base/vine2_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/base/vine2_n.jpg", cfg);
	//entity->emissiveTex = 
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/base/vine2_ao.jpg", cfg);

	//entity->shininess = 0;
	entity->alphaClip = 0.8;
	entity->doubleSided = true;

	entity->position = glm::vec3(0, 0, 0);
	entity->rotation = glm::vec3(0, 0, 0);
	entity->scale = glm::vec3(1);

	entity->parent = entities_lit[0]; // base

	entities_lit.push_back(entity);
}

static void SpawnTiny()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Tiny";
	entity->mesh = MeshUtils::loadObjFile("../assets/models/tiny/tiny.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgRepeat;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/tiny/tiny.jpg", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/tiny/tiny_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/tiny/tiny_n.jpg", cfg);
	entity->emissiveTex = TextureUtils::loadTexture2D("../assets/textures/tiny/tiny_e.jpg", cfg);
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/tiny/tiny_ao.jpg", cfg);

	//entity->shininess = 0;
	//entity->alphaClip = 0.1;
	//entity->doubleSided = false;

	entity->position = glm::vec3(-0.1, 0.9, -0.7);
	entity->rotation = glm::vec3(0, 0, 0);
	entity->scale = glm::vec3(0.5);

	entity->parent = entities_lit[0]; // base

	entity->breathingSpeed = 2;

	//entity->active = true;

	entities_lit.push_back(entity);
}

static void SpawnBear()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Bear";
	entity->mesh = MeshUtils::loadObjFile("../assets/models/figurines/bear.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgRepeat;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/figurines/bear/bear.jpg", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/figurines/bear/bear_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/figurines/bear/bear_n.jpg", cfg);
	entity->emissiveTex = TextureUtils::loadTexture2D("../assets/textures/figurines/bear/bear_e.jpg", cfg);
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/figurines/bear/bear_ao.jpg", cfg);

	//entity->shininess = 0;
	//entity->alphaClip = 0.1;
	//entity->doubleSided = false;

	entity->position = glm::vec3(-0.1, 0.8, 1.5);
	entity->rotation = glm::vec3(-10, 180, 0);
	entity->scale = glm::vec3(0.6);

	entity->parent = entities_lit[0]; // base

	entity->breathingSpeed = 3;

	//entity->active = true;

	entities_lit.push_back(entity);
}

static void SpawnCat()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Cat";
	entity->mesh = MeshUtils::loadObjFile("../assets/models/figurines/cat.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgRepeat;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/figurines/cat/cat.jpg", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/figurines/cat/cat_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/figurines/cat/cat_n.jpg", cfg);
	entity->emissiveTex = TextureUtils::loadTexture2D("../assets/textures/figurines/cat/cat_e.jpg", cfg);
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/figurines/cat/cat_ao.jpg", cfg);

	//entity->shininess = 0;
	//entity->alphaClip = 0.1;
	//entity->doubleSided = false;

	entity->position = glm::vec3(-2.4, 0.8, -0.7);
	entity->rotation = glm::vec3(0, 90, 0);
	entity->scale = glm::vec3(0.6);

	entity->parent = entities_lit[0]; // base

	entity->breathingSpeed = 3.2;

	//entity->active = true;

	entities_lit.push_back(entity);
}

static void SpawnOwl()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Owl";
	entity->mesh = MeshUtils::loadObjFile("../assets/models/figurines/owl.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgRepeat;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/figurines/owl/owl.jpg", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/figurines/owl/owl_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/figurines/owl/owl_n.jpg", cfg);
	entity->emissiveTex = TextureUtils::loadTexture2D("../assets/textures/figurines/owl/owl_e.jpg", cfg);
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/figurines/owl/owl_ao.jpg", cfg);

	//entity->shininess = 0;
	//entity->alphaClip = 0.1;
	//entity->doubleSided = false;

	entity->position = glm::vec3(2.2, 0.8, -0.7);
	entity->rotation = glm::vec3(0, -90, 0);
	entity->scale = glm::vec3(0.6);

	entity->parent = entities_lit[0]; // base

	//entity->active = true;

	entity->breathingSpeed = 3.4;

	entities_lit.push_back(entity);
}

static void SpawnTurtle()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Turtle";
	entity->mesh = MeshUtils::loadObjFile("../assets/models/figurines/turtle.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgRepeat;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/figurines/turtle/turtle.jpg", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/figurines/turtle/turtle_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/figurines/turtle/turtle_n.jpg", cfg);
	entity->emissiveTex = TextureUtils::loadTexture2D("../assets/textures/figurines/turtle/turtle_e.jpg", cfg);
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/figurines/turtle/turtle_ao.jpg", cfg);

	//entity->shininess = 0;
	//entity->alphaClip = 0.1;
	//entity->doubleSided = false;

	entity->position = glm::vec3(0, 0.9, -3.3);
	entity->rotation = glm::vec3(-0.5, 0, 0);
	entity->scale = glm::vec3(0.6);

	entity->parent = entities_lit[0]; // base

	entity->breathingSpeed = 3.6;

	//entity->active = true;

	entities_lit.push_back(entity);
}

static void SpawnPivotGems()
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = "Pivot Gems";

	entity->position = glm::vec3(0, 2.6, -0.7);
	entity->rotation = glm::vec3(0, 0, 0);
	entity->scale = glm::vec3(1);

	entity->parent = entities_lit[0]; // base

	pivots.push_back(entity);
}

static void SpawnGem(std::string name, std::string obj, glm::vec3 pos)
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = name;
	entity->mesh = MeshUtils::loadObjFile("../assets/models/gems/" + obj);
	entity->shader = shader_lit;

	TextureConfig cfg = cfgRepeat;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/gems/white.png", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/gems/gem_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/gems/gem_n.jpg", cfg);
	entity->emissiveTex = TextureUtils::whiteTexture2D();
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/gems/gem_ao.jpg", cfg);

	//entity->shininess = 0;
	//entity->alphaClip = 0.1;
	entity->doubleSided = true;
	entity->opacity = 0.75;

	entity->position = pos;
	entity->rotation = glm::vec3(0, 0, 0);
	entity->scale = glm::vec3(0.5);

	entity->parent = pivots[0]; // pivot gems

	entity->breathingSpeed = 15;

	//entity->active = true;

	entities_alphablend.push_back(entity);
}

static void SpawnGemLight(std::string name, glm::vec3 col)
{
	PointLight* light = LightUtils::createPointLight(name);

	light->setColour(col);
	light->setIntensity(1);
	light->setRange(5);

	light->setPosition({ 0, 0, 0 });

	lights_point.push_back(light);
}

static void SpawnTorch(std::string name, glm::vec3 pos, glm::vec3 rot)
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = name;
	entity->mesh = MeshUtils::loadObjFile("../assets/models/torch/torch.obj");
	entity->shader = shader_lit;

	TextureConfig cfg = cfgRepeat;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/torch/torch.jpg", cfg);
	entity->specularTex = TextureUtils::loadTexture2D("../assets/textures/torch/torch_s.jpg", cfg);
	entity->normalTex = TextureUtils::loadTexture2D("../assets/textures/torch/torch_n.jpg", cfg);
	//entity->emissiveTex =
	entity->aoTex = TextureUtils::loadTexture2D("../assets/textures/torch/torch_ao.jpg", cfg);

	//entity->shininess = 0;
	//entity->alphaClip = 0.1;
	//entity->doubleSided = false;

	entity->position = pos;
	entity->rotation = rot;
	entity->scale = glm::vec3(0.3);

	entity->parent = entities_lit[0]; // base

	//entity->active = true;

	entities_lit.push_back(entity);
}

static void SpawnFire(std::string name, RenderableEntity* parent)
{
	RenderableEntity* entity = new RenderableEntity();

	entity->name = name;
	entity->mesh = MeshUtils::loadObjFile("../assets/models/torch/fire.obj");
	entity->shader = shader_fire;

	TextureConfig cfg = cfgClamp;

	entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/torch/fire.png", cfg);

	//entity->alphaClip = 0.1;
	entity->doubleSided = true;

	entity->position = glm::vec3(-0.1, 4.2, 0.1);
	entity->rotation = glm::vec3(0, 0, 0);
	entity->scale = glm::vec3(0.04);

	entity->parent = parent; // torch

	entity->breathingSpeed = 10;

	//entity->active = true;

	entities_alphablend.push_back(entity);
}

static void SpawnFireLight(std::string name)
{
	PointLight* light = LightUtils::createPointLight(name);

	light->setColour({ 1, 0.5, 0 });
	light->setIntensity(0.5);
	light->setRange(30);

	light->setPosition({ 0, 20, 0 });

	lights_point.push_back(light);
}

static void SpawnSpotlight()
{
	SpotLight* light = LightUtils::createSpotLight("SPOT LIGHT");

	light->setColour({ 1, 1, 1 });
	light->setIntensity(1);
	light->setRange(10);
	light->setInputAngles(20, 40);

	light->setPosition({ 0, 7, 0 });
	light->setDirection({ 0, -1, 0 });

	lights_spot.push_back(light);
}


//LOAD--------------------------------------------------------------------------------
// load() runs AFTER loadShaders()
// Shader* variables are initialized and can be safely used.

static void load_instructions()
{
	// 1. Use factory function to create lights.
	//    The factory function creates the light and adds it to a list for debug drawing purposes

	// 2. Set the light properties
	//    Colour/Color	- Common
	//    Direction		- Directional Light
	//    Position		- Point Light, Spot Light
	//    Range			- Point Light, Spot Light
	//    Angle			- Spot Light

	// 3. Create renderable entities for the scene.
	//
	//		Entity creation flow:
	//		1. Create new RenderableEntity
	//		2. Assign mesh
	//		3. Assign shader (shaders in loadShaders() are safe to use here!)
	//		4. Set material properties
	//		5. Set transformation properties
	//		6. Push to the relevant collection (opaque, alpha-test or alpha-blend)
	//
	// Note: it is your own responsibility to not insert the same entity multiple times.

	// Example
	// RenderableEntity* et1 = new RenderableEntity();
	// et1->mesh = ...;
	// et1->shader = ...;
	// et1->diffuseTex = TextureUtils::loadTexture2D("...", ...);
	// et1->specularTex = TextureUtils::loadTexture2D("...", ...);
	// et1->normalTex = TextureUtils::loadTexture2D("...", ...);
	// et1->emissiveTex = TextureUtils::loadTexture2D("...", ...);
	// et1->shininess = ...;
	// 
	// entities_opaque.push_back(et1);
}

static void LoadHierarchy()
{
	SpawnSunlight();
	// lights_directional[0]

	SpawnBase();
	// entities_lit[0]
		SpawnVine1();
		// entities_lit[1]
		SpawnVine2();
		// entities_lit[2]

		SpawnTiny();
		// entities_lit[3]
		SpawnBear();
		// entities_lit[4]
		SpawnCat();
		// entities_lit[5]
		SpawnOwl();
		// entities_lit[6]
		SpawnTurtle();
		// entities_lit[7]

		SpawnPivotGems();
		// pivots[0]
			SpawnGem("Blue Gem", "blue.obj", glm::vec3(1.6, 0, 0));
			// entities_alphablend[0]
				SpawnGemLight("Blue Gem Light", glm::vec3(0, 0, 1));
				// lights_point[0]

			SpawnGem("Green Gem", "green.obj", glm::vec3(0, 0, -1.6));
			// entities_alphablend[1]
				SpawnGemLight("Green Gem Light", glm::vec3(0, 1, 0));
				// lights_point[1]

			SpawnGem("Purple Gem", "purple.obj", glm::vec3(-1.6, 0, 0));
			// entities_alphablend[2]
				SpawnGemLight("Purple Gem Light", glm::vec3(1, 0, 1));
				// lights_point[2]

			SpawnGem("Red Gem", "red.obj", glm::vec3(0, 0, 1.6));
			// entities_alphablend[3]
				SpawnGemLight("Red Gem Light", glm::vec3(1, 0, 0));
				// lights_point[3]


		SpawnTorch("Torch 1", glm::vec3(-1.6, 0.8, 0.6), glm::vec3(0, 0, 0));
		// entities_lit[8]
			SpawnFire("Fire 1", entities_lit[8]);
			// entities_alphablend[4]
				SpawnFireLight("Fire Light 1");
				// lights_point[4]

		SpawnTorch("Torch 2", glm::vec3(1.6, 0.8, 0.6), glm::vec3(0, 45, 0));
		// entities_lit[9]
			SpawnFire("Fire 2", entities_lit[9]);
			// entities_alphablend[5]
				SpawnFireLight("Fire Light 2");
				// lights_point[5]

		SpawnTorch("Torch 3", glm::vec3(-1.6, 0.8, -2.8), glm::vec3(0, 90, 0));
		// entities_lit[10]
			SpawnFire("Fire 3", entities_lit[10]);
			// entities_alphablend[6]
				SpawnFireLight("Fire Light 3");
				// lights_point[6]

		SpawnTorch("Torch 4", glm::vec3(1.6, 0.8, -2.8), glm::vec3(0, 135, 0));
		// entities_lit[11]
			SpawnFire("Fire 4", entities_lit[11]);
			// entities_alphablend[7]
				SpawnFireLight("Fire Light 4");
				// lights_point[7]


	//SpawnSpotlight();
	// lights_spot[0]
}

void Scene_ASGN::load()
{
	LoadHierarchy();

	CreateShadowMap();

	LoadFBO();
}


//ANIMATIONS--------------------------------------------------------------------------------

static void animation_instructions()
{
	// Get your renderable entity by array indexing
	// and then do update on its position, rotation, etc
	//
	//auto& et = entities_opaque[0];
	//et->position.x = ...;
}

float Wave(float amp, float freq, float axis, float xOffset, float yOffset)
{
	return amp * sin(freq * (axis + xOffset)) + yOffset;
}

static void PivotGemsAnim(float t)
{
	auto& ent = pivots[0];
	float rotateSpeed = 50;
	ent->rotation.y = t * rotateSpeed;
}

static void SpinGemsAnim(float t)
{
	float spinSpeed = 50;

	auto& blueGem = entities_alphablend[0];
	blueGem->rotation.x = t * spinSpeed;
	blueGem->rotation.y = t * spinSpeed;
	blueGem->rotation.z = t * spinSpeed;
	
	auto& greenGem = entities_alphablend[1];
	greenGem->rotation.x = -t * spinSpeed;
	greenGem->rotation.y = -t * spinSpeed;
	greenGem->rotation.z = -t * spinSpeed;
	
	auto& purpleGem = entities_alphablend[2];
	purpleGem->rotation.x = -t * spinSpeed;
	purpleGem->rotation.y = t * spinSpeed;
	purpleGem->rotation.z = -t * spinSpeed;
	
	auto& redGem = entities_alphablend[3];
	redGem->rotation.x = t * spinSpeed;
	redGem->rotation.y = -t * spinSpeed;
	redGem->rotation.z = t * spinSpeed;
}

static void WaveGemsAnim(float t)
{
	float amp = 0.25;
	float freq = 3;
	float pie = 3.14159;

	auto& blueGem = entities_alphablend[0];
	blueGem->position.y = Wave(amp, freq, pie, t, 0);
	
	auto& greenGem = entities_alphablend[1];
	greenGem->position.y = Wave(amp, freq, 0, t, 0);
	
	auto& purpleGem = entities_alphablend[2];
	purpleGem->position.y = Wave(amp, freq, pie, t, 0);
	
	auto& redGem = entities_alphablend[3];
	redGem->position.y = Wave(amp, freq, 0, t, 0);
}

static void SpinFireAnim(float t)
{
	float spinSpeed = 500;

	auto& fire1 = entities_alphablend[4];
	fire1->rotation.y = t * spinSpeed;
	
	auto& fire2 = entities_alphablend[5];
	fire2->rotation.y = -t * spinSpeed;
	
	auto& fire3 = entities_alphablend[6];
	fire3->rotation.y = t * spinSpeed;
	
	auto& fire4 = entities_alphablend[7];
	fire4->rotation.y = -t * spinSpeed;
}

static glm::vec3 GetRainbowColor(float t, float speed, float offset)
{
	float time = t * speed;
	float pie = 3.14159;

	// Normalize time to be between 0 and 1
	time = fmod(time + offset, 1.0f);

	float r = 0.5f + 0.5f * sin(2.0f * pie * time + 0.0f);
	float g = 0.5f + 0.5f * sin(2.0f * pie * time + 2.0f * pie / 3.0f);
	float b = 0.5f + 0.5f * sin(2.0f * pie * time + 4.0f * pie / 3.0f);

	glm::vec3 color = glm::vec3(r, g, b);

	return color;
}

static void RainbowGemsAnim(float t, float speed)
{
	auto& blueGem = entities_alphablend[0];
	blueGem->tint = GetRainbowColor(t, speed, 0.66);

	auto& blueGemLight = lights_point[0];
	blueGemLight->setColor(GetRainbowColor(t, speed, 0.66));

	auto& greenGem = entities_alphablend[1];
	greenGem->tint = GetRainbowColor(t, speed, 0.33);

	auto& greenGemLight = lights_point[1];
	greenGemLight->setColor(GetRainbowColor(t, speed, 0.33));

	auto& purpleGem = entities_alphablend[2];
	purpleGem->tint = GetRainbowColor(t, speed, 0.167);

	auto& purpleGemLight = lights_point[2];
	purpleGemLight->setColor(GetRainbowColor(t, speed, 0.167));

	auto& redGem = entities_alphablend[3];
	redGem->tint = GetRainbowColor(t, speed, 0);

	auto& redGemLight = lights_point[3];
	redGemLight->setColor(GetRainbowColor(t, speed, 0));
}

void Scene_ASGN::update()
{
	float t = App::getTime();
	float dt = App::getDeltaTime();

	PivotGemsAnim(t);
	SpinGemsAnim(t);
	WaveGemsAnim(t);
	RainbowGemsAnim(t, 0.5);
	SpinFireAnim(t);
}


//RENDER--------------------------------------------------------------------------------

static bool debugLights = false;

void Scene_ASGN::draw(CameraBase* camera)
{
	BindFBO();

	glEnable(GL_DEPTH_TEST);

	// lights
	RenderDirectionalLights();
	RenderPointLights();
	RenderSpotLights();
	UpdateLightsParenting();

	// objects
	RenderLitObjects(camera);
	RenderSkybox(camera);
	RenderAlphaBlends(camera);	

	if(debugLights)
	LightDebug::draw(camera);
}

void Scene_ASGN::postDraw(CameraBase* camera)
{
	RenderFBO();
}

void Scene_ASGN::onFrameBufferResized(int width, int height)
{
	fbo->resize(width, height);
}



//INSPECTOR--------------------------------------------------------------------------------

static bool editEntities = false;

static void ImGui_Entity(RenderableEntity* ent, bool opaque)
{
	// Need to push ID to ensure internal IDs used in custom UI are different
	ImGui::PushID(ent);

	if (ImGui::CollapsingHeader(ent->name.c_str(), ImGuiTreeNodeFlags_None))
	{
		RenderableEntity* entt = static_cast<RenderableEntity*>(ent);
			
		ImGui::Indent(10);

		ImGui::Text("Active");
		ImGui::Checkbox("##active", &entt->active);

		if (entt->active)
		{
			ImGui::Text("Position");
			ImGui::DragFloat3("##position", &entt->position[0], 0.1);

			ImGui::Text("Rotation");
			ImGui::DragFloat3("##rotation", &entt->rotation[0], 0.1);

			ImGui::Text("Scale");
			float uniformScale = entt->scale[0]; //x
			if (ImGui::DragFloat("##uniformScale", &uniformScale, 0.1f))
			{
				entt->scale[0] = uniformScale; //x
				entt->scale[1] = uniformScale; //y
				entt->scale[2] = uniformScale; //z
			}

			ImGui::Text("Shininess");
			ImGui::DragFloat("##shininess", &entt->shininess, 0.1);

			ImGui::Text("Tint");
			ImGui::ColorEdit3("##tint", &entt->tint[0]);			

			ImGui::Text("Breathing Speed");
			ImGui::DragFloat("##breathingSpeed", &entt->breathingSpeed, 0.1);

			ImGui::Text("Alpha Clip");
			ImGui::DragFloat("##alphaClip", &entt->alphaClip, 0.1);

			ImGui::Text("Double Sided");
			ImGui::Checkbox("##doubleSided", &entt->doubleSided);

			if (!opaque)
			{
				ImGui::Text("Opacity");
				ImGui::DragFloat("##opacity", &entt->opacity, 0.1);
			}
		}		

		ImGui::Indent(-10);

		ImGui::Separator();

		ImGui::Spacing();
	}

	ImGui::PopID();
}

static void ImGui_Entities()
{
	ImGui::Text("Edit Entities");
	ImGui::Checkbox("##editEntities", &editEntities);

	if (!editEntities) return;

	for(auto ent : entities_lit)
	{
		ImGui_Entity(ent, true);
	}
	for(auto ent : entities_alphablend)
	{
		ImGui_Entity(ent, false);
	}
}

static void ImGui_Textures()
{
	if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent(10);

		ImGui::Text("Diffuse");
		ImGui::Checkbox("##enableDiffuse", &enableDiffuse);

		ImGui::Text("Specular");
		ImGui::Checkbox("##enableSpecular", &enableSpecular);

		ImGui::Text("Normal");
		ImGui::Checkbox("##enableNormal", &enableNormal);

		ImGui::Text("Emissive");
		ImGui::Checkbox("##enableEmissive", &enableEmissive);

		ImGui::Text("Ambient Occlusion");
		ImGui::Checkbox("##enableAO", &enableAO);

		ImGui::Indent(-10);

		ImGui::Separator();

		ImGui::Spacing();
	}
}


static bool editLights = false;

static void ImGui_Lights()
{
	ImGui::Text("Edit Lights");
	ImGui::Checkbox("##editLights", &editLights);

	if (!editLights) return;

	for(auto light : lights_directional)
	{
		LightUtils::imgui_drawControls(light);
	}
	for(auto light : lights_point)
	{
		LightUtils::imgui_drawControls(light);
	}
	for(auto light : lights_spot)
	{
		LightUtils::imgui_drawControls(light);
	}
}

static void ImGui_DebugLights()
{
	ImGui::Text("Debug Lights");
	ImGui::Checkbox("##debugLights", &debugLights);
}


static void ImGui_PostProcess()
{
	ImGui::Text("Enable Post Process");
	ImGui::Checkbox("##EnablePostProcess", &EnablePostProcess);

	if (!EnablePostProcess) return;

	if (ImGui::CollapsingHeader("Spherize", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent(10);

		ImGui::Text("Enable");
		ImGui::Checkbox("##EnableSpherize", &EnableSpherize);

		ImGui::Text("Strength");
		ImGui::DragFloat("##SpherizeStrength", &SpherizeStrength, 0.1);

		ImGui::Text("Zoom Scale");
		ImGui::DragFloat("##ZoomScale", &ZoomScale, 0.1);

		ImGui::Indent(-10);

		ImGui::Separator();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Grayscale", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent(10);

		ImGui::Text("Enable");
		ImGui::Checkbox("##EnableGrayscale", &EnableGrayscale);

		ImGui::Indent(-10);

		ImGui::Separator();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Scanlines", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent(10);

		ImGui::Text("Enable");
		ImGui::Checkbox("##EnableScanlines", &EnableScanlines);

		ImGui::Text("Tiling");
		ImGui::DragInt("##ScanlineTiling", &ScanlineTiling);

		ImGui::Text("Scroll Speed");
		ImGui::DragFloat("##ScanlineScroll", &ScanlineScroll, 0.1);

		ImGui::Indent(-10);

		ImGui::Separator();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Vignette", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent(10);

		ImGui::Text("Enable");
		ImGui::Checkbox("##EnableVignette", &EnableVignette);

		ImGui::Text("Strength");
		ImGui::DragFloat("##VignetteStrength", &VignetteStrength, 0.1);

		ImGui::Text("Color");
		ImGui::ColorEdit3("##VignetteColor", &VignetteColor[0]);

		ImGui::Indent(-10);

		ImGui::Separator();

		ImGui::Spacing();
	}
}


static void ImGui_Shadow()
{
	ImGui::Text("Enable Shadow");
	ImGui::Checkbox("##EnableShadow", &EnableShadow);

	if (!EnableShadow) return;

	if (ImGui::CollapsingHeader("Shadow", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent(10);

		ImGui::Text("Bounds");
		ImGui::DragFloat("##shadow_bounds", &shadow_bounds, 0.1);

		ImGui::Text("Near Plane");
		ImGui::DragFloat("##shadow_near_plane", &shadow_near_plane, 0.1);

		ImGui::Text("Far Plane");
		ImGui::DragFloat("##shadow_far_plane", &shadow_far_plane, 0.1);

		ImGui::Text("Strength");
		ImGui::DragFloat("##ShadowStrength", &ShadowStrength, 0.1);

		ImGui::Text("Bias");
		ImGui::DragFloat("##ShadowBias", &ShadowBias, 0.0001);

		ImGui::Indent(-10);

		ImGui::Separator();

		ImGui::Spacing();
	}
}


#ifdef XBGT2094_ENABLE_IMGUI
void Scene_ASGN::imgui_draw()
{
	ImGui::Text("Assignment");
	ImGui::Separator();
	ImGui::Text("Name: Joshua Yeoh");
	ImGui::Text("ID  : 0135760");
	ImGui::Separator();
	ImGui::PushItemWidth(-1); // make widgets below have full width
	// Put your widget codes here

	ImGui_Entities();

	ImGui::Separator();

	ImGui_Textures();

	ImGui::Separator();

	ImGui_Lights();

	ImGui::Separator();

	ImGui_DebugLights();

	ImGui::Separator();

	ImGui_PostProcess();

	ImGui::Separator();

	ImGui_Shadow();

	ImGui::Separator();

	ImGui::PopItemWidth();
}
#endif