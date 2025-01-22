#include "VulkanWindow.hpp"

namespace Engine {
	VulkanWindow::VulkanWindow(const uint32_t width, const uint32_t height, const char* windowName)
		: width(width), height(height), windowName(windowName)
	{
		initWindow();
	}

	VulkanWindow::~VulkanWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VulkanWindow::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
	}

	bool VulkanWindow::shouldClose()
	{
		return glfwWindowShouldClose(window);
	}

	bool VulkanWindow::tryToCreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		return glfwCreateWindowSurface(instance, window, nullptr, surface);
	}
}