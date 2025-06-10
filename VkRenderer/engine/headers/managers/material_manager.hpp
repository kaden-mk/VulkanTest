#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

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

		void addMaterial(std::string name, Material& material);
		void addTexture(std::string name, std::unique_ptr<VulkanTexture> texture);
		void updateGPUBuffer();

		VulkanTexture* getTexture(const std::string& name) const;
		int getTextureId(const std::string& name);

		int getMaterialId(const std::string& name);
		const std::vector<std::string>& getMaterials() const { return materialOrder; }

		auto getDescriptorInfo() { return buffer.descriptorInfo(); }
		const std::vector<std::string>& getTextures() const { return textureOrder; }
	private:
		std::vector<std::string> materialOrder;
		std::unordered_map<std::string, Material> materials;

		std::vector<std::string> textureOrder;
		std::unordered_map<std::string, std::unique_ptr<VulkanTexture>> textures;

		VulkanDevice& device;
		VulkanBuffer buffer;
	};
}