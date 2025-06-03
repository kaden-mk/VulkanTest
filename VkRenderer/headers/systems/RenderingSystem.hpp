#pragma once

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanObject.h"
#include "VulkanCamera.hpp"
#include "VulkanFrameInfo.hpp"

namespace VkRenderer {
    class RenderingSystem
    {
    public:
        float deltaTime;

        RenderingSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~RenderingSystem();

        RenderingSystem(const RenderingSystem&) = delete;
        RenderingSystem& operator=(const RenderingSystem&) = delete;

        void renderObjects(FrameInfo &frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        VulkanDevice& device;

        std::unique_ptr<VulkanPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };
}