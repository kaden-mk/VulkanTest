#include "VulkanWindow.hpp"

namespace VkRenderer {
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

	void VulkanWindow::setFrameBufferSize(int* width, int* height)
	{
		glfwGetFramebufferSize(window, width, height);
	}

	void VulkanWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}
}