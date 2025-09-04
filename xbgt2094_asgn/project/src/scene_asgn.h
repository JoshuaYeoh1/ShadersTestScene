#pragma once
#include "framework/scenebase.h"
#include "mesh/mesh_utils.h"
#include "shader/shader.h"

class Scene_ASGN : public SceneBase
{
protected:
	void preload() override;
	void load() override;
	void update() override;
	void draw(CameraBase* camera) override;
	void loadShaders() override;

	// Needed for FBO
	void postDraw(CameraBase* camera) override;
	void onFrameBufferResized(int width, int height) override;

#ifdef XBGT2094_ENABLE_IMGUI
	void imgui_draw() override;
#endif
};
