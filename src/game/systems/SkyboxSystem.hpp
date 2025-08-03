#pragma once

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanModel.hpp"
#include "VulkanFrameInfo.hpp"

namespace VkRenderer {
	class SkyboxSystem
	{
	public:
		SkyboxSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SkyboxSystem();

		SkyboxSystem(const SkyboxSystem&) = delete;
		SkyboxSystem& operator=(const SkyboxSystem&) = delete;

		void assignHDRI(const char* path);
		void renderSkybox(FrameInfo &frameInfo);
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		void createCube();

		VulkanDevice& device;
		std::unique_ptr<VulkanModel> cube;

		std::unique_ptr<VulkanPipeline> pipeline;
		VkPipelineLayout pipelineLayout;
	};
}
