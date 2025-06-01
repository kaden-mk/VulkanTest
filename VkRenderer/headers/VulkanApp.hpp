#pragma once

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "VulkanObject.h"
#include "VulkanRenderer.hpp"
#include "VulkanDescriptors.hpp"

namespace VkRenderer {
	class VulkanApp
	{
    public:
        static constexpr int WIDTH = 1920;
        static constexpr int HEIGHT = 1080;
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

        std::unique_ptr<VulkanDescriptorPool> globalPool{};
        VulkanObject::Map objects;
	};
}