#pragma onec

#include "VulkanCamera.hpp"

#include <vulkan/vulkan.h>

namespace VkRenderer {
	struct FrameInfo {
		int frameIndex;
		float deltaTime;
		VkCommandBuffer commandBuffer;
		VulkanCamera& camera;
	};
}