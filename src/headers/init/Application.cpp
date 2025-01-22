#include "Application.hpp"
#include "VulkanRenderer.hpp"

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

	void Application::run()
	{
		// Create a VulkanRenderer object
		Engine::VulkanRenderer renderer;

		mainLoop();
	}

	void Application::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
	}
	void Application::mainLoop()
	{
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}
}