#pragma once

#include "VulkanDevice.hpp"
#include "VulkanModel.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

namespace VkRenderer {
	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class VulkanPipeline 
	{
		public:
			VulkanPipeline(
				VulkanDevice &device,
				const std::string& vertFilepath,
				const std::string& fragFilepath,
				const PipelineConfigInfo& configInfo);
			~VulkanPipeline();

			VulkanPipeline(const VulkanPipeline&) = delete;
			VulkanPipeline& operator=(const VulkanPipeline&) = delete;

			void bind(VkCommandBuffer commandBuffer);
			static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		private:
			static std::vector<char> readFile(const std::string& filepath);

			void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
			void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

			VulkanDevice& device;
			VkPipeline graphicsPipeline;
			VkShaderModule vertShaderModule;
			VkShaderModule fragShaderModule;
	};
}