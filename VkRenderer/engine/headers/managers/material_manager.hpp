#pragma once

#include <iostream>
#include <vector>

#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanDevice.hpp"

namespace VkRenderer {
	const int MAX_MATERIAL_COUNT = 100;

	struct Material {
		uint32_t albedoIndex;
		uint32_t normalIndex;
		//uint32_t roughnessIndex;
	};

	class MaterialManager
	{
	public:
		MaterialManager(VulkanDevice& device);
		~MaterialManager();

		void addMaterial(Material& material);
		void updateGPUBuffer();

		auto getDescriptorInfo() { return buffer.descriptorInfo(); };
	private:
		std::vector<Material> materials;

		VulkanDevice& device;
		VulkanBuffer buffer;
	};
}