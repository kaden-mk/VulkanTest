#pragma once

#include "VulkanCamera.hpp"
#include "VulkanObject.h"

#include <vulkan/vulkan.h>

namespace VkRenderer {
	constexpr int MAX_LIGHTS = 10;

	struct PointLight {
		glm::vec4 position{};
		glm::vec4 color{};
	};

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, 0.02f }; // w is intensity
		PointLight pointLights[MAX_LIGHTS];
		int lightCount;
	};

	struct FrameInfo {
		int frameIndex;
		float deltaTime;
		VkCommandBuffer commandBuffer;
		VulkanCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		VulkanObject::Map& objects;
	};
}