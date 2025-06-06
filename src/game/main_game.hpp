#pragma once

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "VulkanObject.h"
#include "VulkanRenderer.hpp"
#include "VulkanDescriptors.hpp"
#include "movement_controller.hpp"

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
        void initImGui();
        void runImGui(VkCommandBuffer commandBuffer);
        void loadObjects();
        void loadTextures();

        // utility
        bool isToggled(auto key);

        VulkanWindow window{ WIDTH, HEIGHT, WINDOW_NAME };
        VulkanDevice device{ window };
        VulkanRenderer renderer{ window, device };

        MovementController cameraController{ window.getWindow() };

        std::unique_ptr<VulkanDescriptorPool> globalPool{};
        std::vector<std::unique_ptr<VulkanTexture>> textures;
        VulkanObject::Map objects;

        bool showImGui = false;
        int selectedObjectId = -1;

        float nearDistance = 0.1f;
        float farDistance = 10000.f;

        // camera
        VulkanObject viewerObject = VulkanObject::create();
	};
}