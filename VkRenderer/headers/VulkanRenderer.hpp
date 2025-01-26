#pragma once

#include <iostream>
#include <map>
#include <optional>
#include <vector>
#include <array>
#include <stdexcept>
#include <set>
#include <cstdlib>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#include <memory>

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanWindow.hpp"

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
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();

        void initVulkan();
        
        void mainLoop();
       
        void createImageViews();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        VulkanWindow window{WIDTH, HEIGHT, WINDOW_NAME};
        VulkanDevice device{window};
        VulkanSwapChain swapChain{ device, window.getExtent() };
        std::unique_ptr<VulkanPipeline> pipeline;

        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
    };
}