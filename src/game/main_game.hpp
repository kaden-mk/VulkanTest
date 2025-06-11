#pragma once

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "VulkanObject.h"
#include "VulkanRenderer.hpp"
#include "VulkanWorld.hpp"

#include "systems/RenderingSystem.hpp"
#include "systems/PointLightSystem.hpp"

#include "movement_controller.hpp"

#include "managers/material_manager.hpp"

namespace VkRenderer {
 	class Game
	{
    public:
        static constexpr int WIDTH = 1920;
        static constexpr int HEIGHT = 1080;
        static constexpr const char* WINDOW_NAME = "Vulkan";

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
        VulkanWorld world{ window };

        std::unique_ptr<RenderingSystem> renderingSystem;
        std::unique_ptr<PointLightSystem> pointLightSystem;

        MovementController cameraController{ window.getWindow() };

        bool showImGui = false;
        int selectedObjectId = -1;

        VulkanObject viewerObject = VulkanObject::create();
	};
}