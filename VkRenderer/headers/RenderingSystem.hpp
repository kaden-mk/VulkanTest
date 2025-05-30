#pragma once

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanObject.h"
#include "VulkanCamera.hpp"

namespace VkRenderer {
    class RenderingSystem
    {
    public:
        float deltaTime;

        RenderingSystem(VulkanDevice& device, VkRenderPass renderPass);
        ~RenderingSystem();

        RenderingSystem(const RenderingSystem&) = delete;
        RenderingSystem& operator=(const RenderingSystem&) = delete;

        void renderObjects(VkCommandBuffer commandBuffer, std::vector<VulkanObject> &objects, const VulkanCamera& camera);
    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        VulkanDevice& device;

        std::unique_ptr<VulkanPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };
}