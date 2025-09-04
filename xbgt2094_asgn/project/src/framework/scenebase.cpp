#include <glad/glad.h>
#include "scenebase.h"
#include "simplerenderer.h"
#include "../shader/shader_utils.h"
#include "../mesh/debugmesh.h"

// in C++, free variables with static keyword is LOCAL to that CPP file;
// The variables ARE ONLY EXPOSED to that file.
// This means:
// 1. Other CPP files CANNOT refer to these static variables
// 2. Other CPP files CAN have variable with the SAME NAME AND TYPE.
//		Without static keyword, compilation will fail due to
//		"VARTYPE XXX" already defined in YYY.obj

// These variables can be used in class member function,
// so if you're lazy and don't want to declare variables in .h
// and define it in .cpp, you can use this method.

static glm::mat4 identity(1.0f);

static const char* debugV = "#version 330 core\nlayout(location = 0) in vec3 ap;layout(location = 3) in vec3 ac;uniform mat4 m,vp;out vec3 c;void main(){c=ac;gl_Position=vp*m*vec4(ap,1.0f);}";
static const char* debugF = "#version 330 core\nlayout(location = 0) out vec4 f;in vec3 c;void main(){f = vec4(c,1.0);}";

void SceneBase::draw_debug(CameraBase* camera)
{
	static Shader* debugShader = ShaderUtils::createShaderInternal("DEBUG", debugV, debugF);
	static DebugMesh* grid = DebugMeshUtils::makeGrid();
	static DebugMesh* axis = DebugMeshUtils::makeAxis();
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	SimpleRenderer::bindShader(debugShader);
	SimpleRenderer::setShaderProp_Mat4("vp", camera->getMatrixVP());
	SimpleRenderer::setShaderProp_Mat4("m", identity);

	// For thicker debug lines.
	glLineWidth(2.0f);

	grid->draw();

	// We want the axis lines to be drawn on top of grid lines
	// in case their depth values are equal
	// So we change the depth test comparison.
	glDepthFunc(GL_LEQUAL);
	axis->draw();
	glDepthFunc(GL_LESS);	// Revert back to default
	glLineWidth(1.0f);		// Revert back to default

	SimpleRenderer::bindShader(0);
}

void SceneBase::step_init()
{
	preload();
	step_loadShaders();
	load();
}

void SceneBase::step_update()
{
	update();
}

void SceneBase::step_draw(CameraBase* camera)
{
	if (renderDebug)
		draw_debug(camera);

	draw(camera);
}

void SceneBase::step_postDraw(CameraBase* camera)
{
	postDraw(camera);
}

void SceneBase::step_fb_resized(int width, int height)
{
	onFrameBufferResized(width, height);
}