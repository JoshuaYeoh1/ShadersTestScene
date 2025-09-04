#include <glad/glad.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "light_debug.h"
#include "light_utils.h"
#include "../framework/simplerenderer.h"
#include "../shader/shader_utils.h"
#include "../mesh/mesh_utils.h"
#include "../mesh/debugmesh.h"
#include "../camera/camera_base.h"

static const char* lightV = "#version 330 core\nlayout(location = 0) in vec3 ap;layout(location = 3) in vec4 vc;uniform mat4 m,vp;out vec4 vcol;void main(){vcol=vc;gl_Position = vp*m*vec4(ap,1.0f);}";
static const char* lightF = "#version 330 core\nlayout(location = 0) out vec4 f;layout(location = 1) out vec4 f2;uniform vec3 c;in vec4 vcol;void main(){f=vec4(vcol.rgb*c,1.0);f2=vec4(vcol.rgb*c,1.0);}";

std::vector<LightBase*> LightDebug::CURRENT_LIGHTS;

void LightDebug::add(LightBase* light)
{
	CURRENT_LIGHTS.push_back(light);
}

void LightDebug::clear()
{
	LightDebug::CURRENT_LIGHTS.clear();
}

static glm::mat4 getRotation(glm::vec3 dir)
{
	static glm::vec3 dirDefault(0.0f, 0.0f, 1.0f);
	auto dirNorm = glm::normalize(dir);
	auto cross = glm::cross(dirDefault, dirNorm);
	auto dot = glm::dot(dirDefault, dirNorm);

	glm::mat3 rotMat;
	if (dot <= -1.0f) // only when direction is opposite of dirOri,
	{
		glm::vec3 defUp(0.0f, 1.0f, 0.0f); //dirDefault is 0 0 1, so 0 1 0 is safe for rot axis.
		rotMat = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), defUp);
	}
	else
	{
		glm::mat3 skewSymCross = glm::mat3(0.0f, cross.z, -cross.y, -cross.z, 0, cross.x, cross.y, -cross.x, 0.0f);
		float finPart = 1.0f / (1.0f + dot);
		rotMat = glm::mat3(1.0f) + skewSymCross + (skewSymCross * skewSymCross) * finPart;
	}

	return glm::mat4(rotMat);
}

void LightDebug::drawDebug(LightBase* light)
{
	static Mesh* ico = MeshUtils::loadObjFile("../assets/app/models/icosphere.obj");
	static Mesh* tip = MeshUtils::loadObjFile("../assets/app/models/arrowtip.obj");
	static Mesh* body = MeshUtils::loadObjFile("../assets/app/models/arrowbody.obj");

	switch (light->getType())
	{
	case LightType::DIRECTIONAL:
	{
		static glm::mat4 arrowBodyMatrix = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, -1.0f }) * glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 0.8f });

		DirectionalLight* d = static_cast<DirectionalLight*>(light);

		auto rot = getRotation(d->direction);
		SimpleRenderer::setShaderProp_Mat4("m", rot * arrowBodyMatrix);
		SimpleRenderer::drawMesh(body);
		SimpleRenderer::setShaderProp_Mat4("m", rot);
		SimpleRenderer::drawMesh(tip);
	}
	break;
	case LightType::POINT:
	{
		static DebugMesh* wireSphere = DebugMeshUtils::makeWireSphere(glm::vec3(1.0F));

		PointLight* p = static_cast<PointLight*>(light);
		glm::mat4 tr = p->getTransformMatrix();
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(p->getRange()));
		SimpleRenderer::setShaderProp_Mat4("m", tr);
		SimpleRenderer::drawMesh(ico);
		SimpleRenderer::setShaderProp_Mat4("m", tr * scale);
		wireSphere->draw();
	}
	break;
	case LightType::SPOT:
	{
		SpotLight* s = static_cast<SpotLight*>(light);
		float range = s->getRange();
		glm::mat4 tr = s->getTransformMatrix();
		auto rot = getRotation(s->Directional::getDirection());

		static DebugMeshCone* oCone = DebugMeshUtils::makeCone(s->getInput_OuterAngle(), range, glm::vec3(1.0f));
		static DebugMeshCone* iCone = DebugMeshUtils::makeCone(s->getInput_InnerAngle(), range, glm::vec3(0.6f));

		float innerAngle = s->getInput_InnerAngle();
		float outerAngle = s->getInput_OuterAngle();

		oCone->changeParams(s->getInput_OuterAngle(), range);
		iCone->changeParams(std::min(innerAngle, outerAngle), range);

		SimpleRenderer::setShaderProp_Mat4("m", tr);
		SimpleRenderer::drawMesh(ico);

		SimpleRenderer::setShaderProp_Mat4("m", tr * rot);
		oCone->draw();
		iCone->draw();
	}
	break;
	}
}

void LightDebug::draw(CameraBase* camera)
{
	GLint currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

	static Shader* shaderProgram = ShaderUtils::createShaderInternal("LIGHTDEBUG", lightV, lightF);

	SimpleRenderer::bindShader(shaderProgram);

	SimpleRenderer::setShaderProp_Mat4("vp", camera->getMatrixVP());

	for (LightBase* light : CURRENT_LIGHTS)
	{
		SimpleRenderer::setShaderProp_Vec3("c", light->getColour());
		drawDebug(light);
	}

	glUseProgram(currentProgram);
}