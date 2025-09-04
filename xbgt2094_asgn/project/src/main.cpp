#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

#include "camera/camera_flying.h"
#include "scene_asgn.h"

const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 768;
const char* WINDOW_TITLE = "XBGT2094 Assignment";

CameraBase* camera;
SceneBase* scene;

void framebuffer_size_callback(int width, int height);
void mouse_button_callback(int button, int action);
void scroll_callback(double xoffset, double yoffset);
void key_callback(int key, int scancode, int action);

int main(void)
{
	int result = App::init(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);

	if (!result)
		exit(EXIT_FAILURE);

	App::setFramebufferSizeCallback(framebuffer_size_callback);
	App::setMouseButtonCallback(mouse_button_callback);
	App::setScrollCallback(scroll_callback);
	App::setKeyCallback(key_callback);

	// Scene initialization
	// ------------------------------------------------------------
	scene = new Scene_ASGN();
	scene->step_init();
	// ------------------------------------------------------------

	// Camera initialization
	// ------------------------------------------------------------
	float fov = 60.0f, zNear = 0.1f, zFar = 1000.0f;

	CameraFlying* cfly;
	camera = cfly = new CameraFlying();
	camera->setNearClip(zNear);
	camera->setFarClip(zFar);
	camera->setProjectionType(CameraProjectionType::PERSPECTIVE);
	camera->setFieldOfView(fov);
	cfly->setPosition(0.0f, 3.0f, 5.0f);
	cfly->lookAtPoint(0.0f, 2.0f, 0.0f);

	auto vSize = App::getViewportSize();
	camera->setViewportSize(vSize.x, vSize.y);
	glViewport(0, 0, vSize.x, vSize.y);

	// ------------------------------------------------------------
	// Default Render Settings
	// ------------------------------------------------------------
	glClearColor(0.125f, 0.125f, 0.125f, 1.0f);
	glEnable(GL_DEPTH_TEST);						// Enable Depth Testing
	glEnable(GL_CULL_FACE);							// Cull Back Faces
	glFrontFace(GL_CCW);							// Set Front Face as Counter Clockwise

	std::cout << "Press F1 to reload shaders" << std::endl;

	// Application Loop
	// ------------------------------------------------------------
	while (App::isRunning())
	{
		App::processTime();
		App::processInput();

		camera->update(App::getDeltaTime());
		scene->step_update();

		// Clear the colour and depth buffers before drawing this frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene->step_draw(camera);
		scene->step_postDraw(camera);

		App::beginGUI();
#ifdef XBGT2094_ENABLE_IMGUI
		scene->imgui_draw();
#endif
		App::endGUI();

		App::display();
	}

	App::cleanup();

	exit(EXIT_SUCCESS);
}

#pragma region GLFW Callbacks

void framebuffer_size_callback(int width, int height)
{
	// make sure the viewport matches the new framebuffer dimensions
	glViewport(0, 0, width, height);

	// recalculate projection values since viewport dimensions has changed.
	camera->setViewportSize(width, height);

	// In later weeks (Framebuffer) this is needed
	scene->step_fb_resized(width, height);
}

void mouse_button_callback(int button, int action)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			camera->onLeftMouseDown();
			if (camera->shouldConsumeCursor())
			{
				App::setCursorVisible(false);
			}
		}
		else // GLFW_RELEASE
		{
			camera->onLeftMouseUp();
			if (camera->shouldReleaseCursor())
			{
				App::setCursorVisible(true);
			}
		}
	}
}

void scroll_callback(double xoffset, double yoffset)
{
	camera->onMouseScrolled(xoffset, yoffset);
}

void key_callback(int key, int scancode, int action)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		bool cursorVisible = App::isCursorVisible();

		if (!cursorVisible)
		{
			App::setCursorVisible(true);
			auto sSize = App::getViewportSize();
			App::setCursorPosition(sSize.x * 0.5f, sSize.y * 0.5f);

			camera->cancelInput();
		}
		else
		{
			App::quit();
		}
	}

	if (key == GLFW_KEY_F1 && action == GLFW_RELEASE) {
		printf("Reloading shaders...\n\n");
		scene->step_loadShaders();
	}
}

#pragma endregion