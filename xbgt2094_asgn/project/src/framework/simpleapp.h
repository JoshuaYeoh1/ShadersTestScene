#pragma once
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_int2.hpp>

typedef void (*AppFramebufferSizeFunc)(int width, int height);
typedef void (*AppMouseButtonFunc)(int button, int action);
typedef void (*AppScrollFunc)(double xoffset, double yoffset);
typedef void (*AppKeyFunc)(int key, int scancode, int action);

class App
{
	// Allow main() entry function to call private member functions
	friend int main();

	// All functions are static because internally it uses
	// the singleton.

	// Don't want to allow anyone else to be able to create instances.
	// App* app = new App();	// error
	// App app;					// error
	App() = delete;

private:

	static int init(int width, int height, const char* title);

	static void processTime();
	static void processInput();

	static void setFramebufferSizeCallback(AppFramebufferSizeFunc callback);
	static void setMouseButtonCallback(AppMouseButtonFunc callback);
	static void setScrollCallback(AppScrollFunc callback);
	static void setKeyCallback(AppKeyFunc callback);

	static void display();
	static void cleanup();
	static bool isRunning();

	static void beginGUI();
	static void endGUI();

public:

	static float getTime();
	static float getDeltaTime();
	static glm::vec2 getMousePosition();
	static glm::ivec2 getViewportSize();

	static bool isMouseButtonDown(int mouseCode);
	static bool isMouseButtonUp(int mouseCode);
	static bool isMouseButtonPressing(int mouseCode);

	static bool isKeyDown(int keyCode);
	static bool isKeyUp(int keyCode);
	static bool isKeyPressing(int keyCode);

	static void quit();

	static bool isCursorVisible();
	static void setCursorVisible(bool visible);
	static void setCursorPosition(int x, int y);
};
