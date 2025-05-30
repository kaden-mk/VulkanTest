#pragma once

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "VulkanObject.h"
#include "VulkanRenderer.hpp"

namespace VkRenderer {
	class VulkanApp
	{
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        static constexpr const char* WINDOW_NAME = "Vulkan";

        float deltaTime;

        VulkanApp();
        ~VulkanApp();

        VulkanApp(const VulkanApp&) = delete;
        VulkanApp& operator=(const VulkanApp&) = delete;

        void run();
    private:
        void loadObjects();

        VulkanWindow window{ WIDTH, HEIGHT, WINDOW_NAME };
        VulkanDevice device{ window };
        VulkanRenderer renderer{ window, device };

        std::vector<VulkanObject> objects;
	};
}