#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "RenderingSystem.hpp"

namespace VkRenderer {
	struct SimplePushConstantData {
		glm::mat4 transform{ 1.f };
		alignas(16) glm::vec3 color;
	};

	RenderingSystem::RenderingSystem(VulkanDevice& device, VkRenderPass renderPass) : device{ device } {
		createPipelineLayout();
		createPipeline(renderPass);
	}

	RenderingSystem::~RenderingSystem()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void RenderingSystem::createPipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void RenderingSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		pipeline = std::make_unique<VulkanPipeline>(device, "VkRenderer/shaders/vert.spv", "VkRenderer/shaders/frag.spv", pipelineConfig);
	}

	void RenderingSystem::renderObjects(VkCommandBuffer commandBuffer, std::vector<VulkanObject> &objects, float deltaTime)
	{
		pipeline->bind(commandBuffer);

		for (auto& object : objects) {
			float y = 1.f * deltaTime;
			float x = y / 2;
			float z = x / 2;

			object.transform.rotation.y = glm::mod(object.transform.rotation.y + y, glm::two_pi<float>());
			object.transform.rotation.x = glm::mod(object.transform.rotation.x + x, glm::two_pi<float>());
			object.transform.rotation.z = glm::mod(object.transform.rotation.z + z, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.color = object.color;
			push.transform = object.transform.mat4();

			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
			object.model->bind(commandBuffer);
			object.model->draw(commandBuffer);
		}
	}
}