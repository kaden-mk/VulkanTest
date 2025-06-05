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

	void VulkanWindow::updateTitle(float fps)
	{
		std::stringstream title;
		title << "Vulkan - FPS: " << fps;

		glfwSetWindowTitle(window, title.str().c_str());
	}
}