#pragma once

#include <iostream>
#include <vector>

#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanDevice.hpp"

namespace VkRenderer {
	struct Material {
		std::vector<VulkanTexture> textures;
	};

	class MaterialManager
	{
	public:
		MaterialManager(VulkanDevice& device);
		~MaterialManager();

		void addMaterial(Material& material);
		void updateGPUBuffer();
	private:
		std::vector<Material> materials;

		VulkanDevice& device;
		VulkanBuffer buffer;
	};
}