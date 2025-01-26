#pragma once

#include "VulkanDevice.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

namespace VkRenderer {
	struct PipelineConfigInfo {
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class VulkanPipeline 
	{
		public:
			VulkanPipeline(VulkanDevice &device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
			~VulkanPipeline();

			VulkanPipeline(const VulkanPipeline&) = delete;
			void operator=(const VulkanPipeline&) = delete;

			void bind(VkCommandBuffer commandBuffer);
			static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
		private:
			static std::vector<char> readFile(const std::string& filepath);

			void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, PipelineConfigInfo configInfo);
			void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

			VulkanDevice& device;
			VkPipeline graphicsPipeline;
			VkShaderModule vertShaderModule;
			VkShaderModule fragShaderModule;
	};
}