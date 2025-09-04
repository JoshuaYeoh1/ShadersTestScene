#pragma once
#include <GLFW/glfw3.h>
#include "simpleapp.h"
#include "../camera/camera_base.h"

// READ!
// Scene lifecycle:
//		Creation:
//			1. Scene ctor
//			2. initialize() // cannot be overridden
//				a. preload()			// MUST BE overridden by subclass
//				b. loadShaders()		// MUST BE overridden by subclass
//
//		Application loop:
//			1. update()	// MUST BE overridden by subclass
//			2. draw()	// MUST BE overridden by subclass
//
// Callbacks:
//		On GLFWwindow resizing:		onFramebufferResized()		// OPTIONALLY overridden by subclass, required for later weeks.
//		On 'F1' key pressed:		loadShaders() 				// MUST BE overridden by subclass

class SceneBase
{
private:
	void draw_debug(CameraBase* camera);

protected:
	bool renderDebug = false;

	virtual void preload() = 0;
	virtual void load() = 0;
	virtual void update() = 0;
	virtual void draw(CameraBase* camera) = 0;
	virtual void loadShaders() = 0;

	virtual void onFrameBufferResized(int width, int height) {}
	virtual void postDraw(CameraBase* camera) {}
public:

	void step_init();

	// inlined because called in step_init()
	inline void step_loadShaders()
	{
		loadShaders();
	}

	void step_update();
	void step_draw(CameraBase* camera);

	void step_postDraw(CameraBase* camera);
	void step_fb_resized(int width, int height);

#ifdef XBGT2094_ENABLE_IMGUI
	virtual void imgui_draw() {}
#endif
};