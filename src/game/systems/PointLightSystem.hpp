#pragma once

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanObject.h"
#include "VulkanCamera.hpp"
#include "VulkanFrameInfo.hpp"

namespace VkRenderer {
    class PointLightSystem
    {
    public:
        float deltaTime;

        PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void render(FrameInfo& frameInfo);
        void update(FrameInfo& frameInfo, GlobalUbo& ubo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        VulkanDevice& device;

        std::unique_ptr<VulkanPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };
}