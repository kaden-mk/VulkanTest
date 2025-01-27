#pragma once

#include <iostream>
#include <map>
#include <optional>
#include <vector>
#include <array>
#include <stdexcept>
#include <set>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <algorithm> 
#include <fstream>
#include <memory>
#include <cassert>

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanWindow.hpp"
#include "VulkanModel.hpp"

namespace VkRenderer {
    static class VulkanRenderer
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        static constexpr const char* WINDOW_NAME = "Vulkan";

        VulkanRenderer();
        ~VulkanRenderer();

        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        void run();
    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void freeCommandBuffers();
        void createCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

        void initVulkan();
        void mainLoop();

        VulkanWindow window{WIDTH, HEIGHT, WINDOW_NAME};
        VulkanDevice device{window};
        std::unique_ptr<VulkanSwapChain> swapChain;
        std::unique_ptr<VulkanPipeline> pipeline;
        std::unique_ptr<VulkanModel> model;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
    };
}