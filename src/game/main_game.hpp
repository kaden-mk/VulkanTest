#pragma once

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "VulkanObject.h"
#include "VulkanRenderer.hpp"
#include "VulkanDescriptors.hpp"

namespace VkRenderer {
	class Game
	{
    public:
        static constexpr int WIDTH = 1920;
        static constexpr int HEIGHT = 1080;
        static constexpr const char* WINDOW_NAME = "Vulkan";

        float deltaTime;

        Game();
        ~Game();

        Game(const Game&) = delete;
        Game& operator=(const Game&) = delete;

        void run();
    private:
        void loadObjects();
        void loadTextures();

        VulkanWindow window{ WIDTH, HEIGHT, WINDOW_NAME };
        VulkanDevice device{ window };
        VulkanRenderer renderer{ window, device };

        std::unique_ptr<VulkanDescriptorPool> globalPool{};
        std::vector<std::unique_ptr<VulkanTexture>> textures;
        VulkanObject::Map objects;
	};
}