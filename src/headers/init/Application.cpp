#include "Application.hpp"

namespace Engine {
	Application::Application(const uint32_t width, const uint32_t height, const char* windowName)
		: width(width), height(height), windowName(windowName)
	{
		initWindow();
	}

	Application::~Application()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Application::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
	}

	bool Application::shouldClose()
	{
		return glfwWindowShouldClose(window);
	}

	bool Application::tryToCreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		return glfwCreateWindowSurface(instance, window, nullptr, surface);
	}
}