#include "simpleapp.h"
#include <map>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>

#ifdef XBGT2094_ENABLE_IMGUI

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

constexpr int IMGUI_INSPECTOR_WIDTH = 256;

void imgui_init(GLFWwindow* window);
void imgui_cleanup();
void imgui_begin();
void imgui_end();

#endif

static float currentTime = 0.0f;
static float deltaTime = 0.0f;
static glm::vec2 mousePosition;
static glm::ivec2 viewportSize;
static std::map<int, bool> k0, k1, m0, m1, *kpptr, *kcptr, *mpptr, *mcptr;
static std::map<int, bool>::iterator i;
static GLFWwindow* win;

static AppFramebufferSizeFunc funcptr_fbs;
static AppMouseButtonFunc funcptr_mb;
static AppScrollFunc funcptr_s;
static AppKeyFunc funcptr_k;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void App::setFramebufferSizeCallback(AppFramebufferSizeFunc callback)
{
	funcptr_fbs = callback;
}

void App::setMouseButtonCallback(AppMouseButtonFunc callback)
{
	funcptr_mb = callback;
}

void App::setScrollCallback(AppScrollFunc callback)
{
	funcptr_s = callback;
}

void App::setKeyCallback(AppKeyFunc callback)
{
	funcptr_k = callback;
}

void App::beginGUI()
{
#ifdef XBGT2094_ENABLE_IMGUI
	imgui_begin();
#endif

}

void App::endGUI()
{
#ifdef XBGT2094_ENABLE_IMGUI
	imgui_end();
#endif

}

void App::quit()
{
	glfwSetWindowShouldClose(win, GLFW_TRUE);
}

int App::init(int width, int height, const char* title)
{
	glfwSetErrorCallback([](int error, const char* description) { fprintf(stderr, "Error: %s\n", description); });

	if (!glfwInit())
		return 0;

	// Set OpenGL context on GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	win = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwSetWindowSizeLimits(win, 768, 768, GLFW_DONT_CARE, GLFW_DONT_CARE);

	if (win == nullptr)
	{
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent(win);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 0;
	}

	// Set callbacks
	glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
	glfwSetCursorPosCallback(win, mouse_cursor_callback);
	glfwSetMouseButtonCallback(win, mouse_button_callback);
	glfwSetScrollCallback(win, scroll_callback);
	glfwSetKeyCallback(win, key_callback);

	// Make window refresh rate to follow screen refresh rate
	glfwSwapInterval(1);

	m1 = m0 = {
	  { GLFW_MOUSE_BUTTON_LEFT, false },
	  { GLFW_MOUSE_BUTTON_RIGHT, false },
	  { GLFW_MOUSE_BUTTON_MIDDLE, false }
	};

	kpptr = &k0;
	kcptr = &k1;

	mpptr = &m0;
	mcptr = &m1;

#ifdef XBGT2094_ENABLE_IMGUI
	width -= IMGUI_INSPECTOR_WIDTH;
#endif

	viewportSize.x = width;
	viewportSize.y = height;

#ifdef XBGT2094_ENABLE_IMGUI
	imgui_init(win);
#endif

	return 1;
}

void App::processTime()
{
	float now = glfwGetTime();
	deltaTime = now - currentTime;
	currentTime = now;
}

float App::getTime()
{
	return currentTime;
}

float App::getDeltaTime()
{
	return deltaTime;
}

glm::vec2 App::getMousePosition()
{
	return mousePosition;
}

glm::ivec2 App::getViewportSize()
{
	return viewportSize;
}

void App::processInput()
{
	auto t = kcptr;
	kcptr = kpptr;
	kpptr = t;

	t = mcptr;
	mcptr = mpptr;
	mpptr = t;

	for (auto& i : *kcptr) i.second = glfwGetKey(win, i.first);
	for (auto& i : *mcptr) i.second = glfwGetMouseButton(win, i.first);
}

bool App::isMouseButtonDown(int mouseCode)
{
	auto& mc = *mcptr;
	auto& mp = *mpptr;

	i = mc.find(mouseCode);

	if (i != mc.end())
	{
		return i->second && !mp[mouseCode];
	}
	else return false;
}

bool App::isMouseButtonUp(int mouseCode)
{
	auto& mc = *mcptr;
	auto& mp = *mpptr;

	i = mc.find(mouseCode);

	if (i != mc.end())
	{
		return !(i->second) && mp[mouseCode];
	}
	else return false;
}

bool App::isMouseButtonPressing(int mouseCode)
{
	auto& mc = *mcptr;
	auto& mp = *mpptr;

	i = mc.find(mouseCode);
	if (i != mc.end())
	{
		return i->second;
	}
	else return false;
}

bool App::isKeyDown(int keyCode)
{
	auto& kc = *kcptr;
	auto& kp = *kpptr;

	i = kc.find(keyCode);

	if (i == kc.end())
	{
		kc.emplace(keyCode, false);
		kp.emplace(keyCode, false);
		return false;
	}
	else
	{
		return i->second && !kp[keyCode];
	}
}

bool App::isKeyUp(int keyCode)
{
	auto& kc = *kcptr;
	auto& kp = *kpptr;

	i = kc.find(keyCode);

	if (i == kc.end())
	{
		kc.emplace(keyCode, false);
		kp.emplace(keyCode, false);
		return false;
	}
	else
	{
		return !(i->second) && kp[keyCode];
	}
}

bool App::isKeyPressing(int keyCode)
{
	auto& kc = *kcptr;
	auto& kp = *kpptr;

	i = kc.find(keyCode);

	if (i == kc.end())
	{
		kc.emplace(keyCode, false);
		kp.emplace(keyCode, false);
		return false;
	}
	else
	{
		return i->second;
	}
}

bool App::isCursorVisible()
{
	return glfwGetInputMode(win, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

void App::setCursorVisible(bool visible)
{
		App::processTime();
	glfwSetInputMode(win, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);


#ifdef XBGT2094_ENABLE_IMGUI
	ImGuiIO& io = ImGui::GetIO();

	if(visible)
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	else
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

#endif
}

void App::setCursorPosition(int x, int y)
{
	glfwSetCursorPos(win, x, y);
}

void App::display()
{
	glfwSwapBuffers(win);
	glfwPollEvents();
}

void App::cleanup()
{
#ifdef XBGT2094_ENABLE_IMGUI
	imgui_cleanup();
#endif

	glfwDestroyWindow(win);
	glfwTerminate();
}

bool App::isRunning()
{
	return !glfwWindowShouldClose(win);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0) return;

#ifdef XBGT2094_ENABLE_IMGUI
	width -= IMGUI_INSPECTOR_WIDTH;
#endif

	auto& v = viewportSize;
	v.x = width;
	v.y = height;

	if (funcptr_fbs)
		funcptr_fbs(width, height);
}

static void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	float ypos_invert = viewportSize.y - ypos;

	auto& v = mousePosition;
	v.x = xpos;
	v.y = ypos_invert;
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
#ifdef XBGT2094_ENABLE_IMGUI
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(button, action == GLFW_PRESS);

	if (!io.WantCaptureMouse)
	{
#endif
		if (funcptr_mb)
			funcptr_mb(button, action);
#ifdef XBGT2094_ENABLE_IMGUI
	}
#endif
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (funcptr_s)
		funcptr_s(xoffset, yoffset);

}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (funcptr_k)
		funcptr_k(key, scancode, action);
}


#pragma region Dear ImGui

#ifdef XBGT2094_ENABLE_IMGUI
void imgui_init(GLFWwindow* win)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = 0;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0x12 / 255.0f, 0x44 / 255.0f, 0x20 / 255.0f, 1.0f);

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(win, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void imgui_cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void imgui_begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	const auto& screenSize = App::getViewportSize();

	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->WorkSize.x, main_viewport->WorkPos.y), ImGuiCond_Always, ImVec2(1, 0));
	ImGui::SetNextWindowSize(ImVec2(IMGUI_INSPECTOR_WIDTH, screenSize.y), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin("Inspector", 0, window_flags);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
}

void imgui_end()
{
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
#endif

#pragma endregion