#include "SkyboxSystem.hpp"

namespace VkRenderer {

	struct SkyboxPushConstantData {
		glm::mat4 proj;
		glm::mat4 view;
		glm::vec3 sunDirection;
		float sunSize;
	};

	SkyboxSystem::SkyboxSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: device{ device } {
		cube = VulkanModel::createModelFromFile(device, "assets/models/cube.obj");

		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SkyboxSystem::~SkyboxSystem() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void SkyboxSystem::assignHDRI(const char* path) {
		// Placeholder: Load cubemap texture later
		// VulkanTexture::loadCubemap(device, path);
	}

	void SkyboxSystem::renderSkybox(FrameInfo& frameInfo) {
		VkCommandBuffer commandBuffer = frameInfo.commandBuffer;
		VulkanCamera camera = frameInfo.camera;

		pipeline->bind(commandBuffer);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		SkyboxPushConstantData push{};
		push.proj = camera.getProjection();

		glm::mat4 view = camera.getView();
		view[3] = glm::vec4(0, 0, 0, 1);
		push.view = view;

		auto& lightingData = GetLightingData();

		push.sunDirection = lightingData.sunDirection;
		push.sunSize = lightingData.sunSize;

		vkCmdPushConstants(
			commandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(SkyboxPushConstantData),
			&push
		);

		cube.get()->bind(commandBuffer);
		cube.get()->draw(commandBuffer);
	}

	void SkyboxSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SkyboxPushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");
	}

	void SkyboxSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;       
		pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;    

		pipeline = std::make_unique<VulkanPipeline>(
			device,
			"assets/shaders/compiled/skybox_vert.spv",
			"assets/shaders/compiled/skybox_frag.spv",
			pipelineConfig
		);
	}
}