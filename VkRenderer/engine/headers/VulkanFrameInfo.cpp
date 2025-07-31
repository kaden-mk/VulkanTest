#include "VulkanFrameInfo.hpp"

namespace VkRenderer {
	LightingData& GetLightingData()
	{
		static LightingData instance{
			glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f)),
			glm::radians(2.5f),
			10.f
		};

		return instance;
	}
}