#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

namespace VkRenderer {
	class VulkanWindow
	{
		public:
			VulkanWindow(const uint32_t width, const uint32_t height, const char* windowName);
			~VulkanWindow();

			bool shouldClose();
			
			VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; };

			void setFrameBufferSize(int* width, int* height);
			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		private:
			const uint32_t width;
			const uint32_t height;
			const char* windowName;
			GLFWwindow* window;

			void initWindow();
	};
}