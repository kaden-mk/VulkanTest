#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <map>

#include "PointLightSystem.hpp"

namespace VkRenderer {
	struct PointLightPushConstant {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	PointLightSystem::PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device{ device } {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	PointLightSystem::~PointLightSystem()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstant);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VulkanPipeline::defaultPipelineConfigInfo(pipelineConfig);
		VulkanPipeline::enableAlphaBlending(pipelineConfig);
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		pipeline = std::make_unique<VulkanPipeline>(device, "assets/shaders/compiled/point_light_vert.spv", "assets/shaders/compiled/point_light_frag.spv", pipelineConfig);
	}

	void PointLightSystem::render(FrameInfo& frameInfo)
	{
		VkCommandBuffer commandBuffer = frameInfo.commandBuffer;
		VulkanCamera camera = frameInfo.camera;

		std::map<float, VulkanObject::id_t> sorted;
		for (auto& kv : frameInfo.objects) {
			auto& object = kv.second;

			if (object.pointLight == nullptr)
				continue;

			auto offset = frameInfo.camera.getPosition() - object.transform.translation;
			float disSquared = glm::dot(offset, offset);
			sorted[disSquared] = object.getId();
		}

		pipeline->bind(commandBuffer);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
			auto& object = frameInfo.objects.at(it->second);

			PointLightPushConstant push{};
			push.position = glm::vec4(object.transform.translation, 1.f);
			push.color = glm::vec4(object.color, object.pointLight->lightIntensity);
			push.radius = object.transform.scale.x;
			
			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstant),
				&push
			);

			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}

	void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
	{
		int lightIndex = 0;
		for (auto& kv : frameInfo.objects) {
			auto& object = kv.second;
			if (object.pointLight == nullptr)
				continue;

			assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

			ubo.pointLights[lightIndex].position = glm::vec4(object.transform.translation, 1.f);
			ubo.pointLights[lightIndex].color = glm::vec4(object.color, object.pointLight->lightIntensity);
			lightIndex++;
		}

		ubo.lightCount = lightIndex;
	}
}