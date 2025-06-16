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
		const std::vector<Material>& getMaterials() const { return materialOrder; }
		const std::unordered_map<std::string, size_t> getMaterialItems() const { return materials; }

		auto getDescriptorInfo() { return buffer.descriptorInfo(); }
		const std::vector<std::unique_ptr<VulkanTexture>>& getTextures() const { return textureOrder; }
		const std::unordered_map<std::string, size_t>& getTextureItems() const { return textures; }
	private:
		std::vector<Material> materialOrder;
		std::unordered_map<std::string, size_t> materials;

		std::vector<std::unique_ptr<VulkanTexture>> textureOrder;
		std::unordered_map<std::string, size_t> textures;

		VulkanDevice& device;
		VulkanBuffer buffer;
	};
}