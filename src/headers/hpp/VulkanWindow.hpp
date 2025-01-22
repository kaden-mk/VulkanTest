#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

namespace Engine {
	class VulkanWindow
	{
		public:
			VulkanWindow(const uint32_t width, const uint32_t height, const char* windowName);
			~VulkanWindow();

			bool shouldClose();
			bool tryToCreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		private:
			const uint32_t width;
			const uint32_t height;
			const char* windowName;
			GLFWwindow* window;

			void initWindow();
	};
}