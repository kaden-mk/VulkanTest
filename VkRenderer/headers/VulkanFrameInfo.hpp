#pragma once

#include "VulkanCamera.hpp"
#include "VulkanObject.h"

#include <vulkan/vulkan.h>

namespace VkRenderer {
	struct FrameInfo {
		int frameIndex;
		float deltaTime;
		VkCommandBuffer commandBuffer;
		VulkanCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		VulkanObject::Map& objects;
	};
}