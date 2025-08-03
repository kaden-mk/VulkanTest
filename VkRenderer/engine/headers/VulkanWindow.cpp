#include "VulkanWindow.hpp"

namespace VkRenderer {
	VulkanWindow::VulkanWindow(int width, int height, const char* windowName)
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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		centerWindow();
	}

	void VulkanWindow::centerWindow()
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		int screenSpaceWidth;
		int screenSpaceHeight;
		glfwGetWindowSize(window, &screenSpaceWidth, &screenSpaceHeight);
		glfwSetWindowPos(
			window,
			mode->width / 2 - screenSpaceWidth / 2,
			mode->height / 2 - screenSpaceHeight / 2
		);

		//glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		glfwMakeContextCurrent(window);
	}

	void VulkanWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto vkWindow = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		vkWindow->framebufferResized = true;
		vkWindow->width = width;
		vkWindow->height = height;
	}

	bool VulkanWindow::shouldClose()
	{
		return glfwWindowShouldClose(window);
	}

	void VulkanWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}
}