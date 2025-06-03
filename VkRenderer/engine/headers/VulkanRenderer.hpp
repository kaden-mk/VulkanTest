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
#include <chrono>

#include "VulkanDevice.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanWindow.hpp"

namespace VkRenderer {
    static class VulkanRenderer
    {
    public:
        float deltaTime;

        VulkanRenderer(VulkanWindow& windowToSet, VulkanDevice& deviceToSet);
        ~VulkanRenderer();

        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
        float getAspectRatio() const { return swapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when it's not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();

        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;
    private:
        void freeCommandBuffers();
        void createCommandBuffers();
        void recreateSwapChain();

        VulkanWindow& window;
        VulkanDevice& device;
        std::unique_ptr<VulkanSwapChain> swapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted = false;
    };
}