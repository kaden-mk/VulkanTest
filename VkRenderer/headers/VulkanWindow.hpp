#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <sstream>

namespace VkRenderer {
	class VulkanWindow
	{
		public:
			VulkanWindow(int width, int height, const char* windowName);
			~VulkanWindow();

			bool shouldClose();
			
			VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; };
			bool wasWindowResized() { return framebufferResized; };

			void resetWindowResizedFlag() { framebufferResized = false; };
			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
			void updateTitle(float fps);

			GLFWwindow* getWindow() const { return window; };
		private:
			int width;
			int height;
			bool framebufferResized = false;

			const char* windowName;
			GLFWwindow* window;

			void initWindow();
			static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	};
}