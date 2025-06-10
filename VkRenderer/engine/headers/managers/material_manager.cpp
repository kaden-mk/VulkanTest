/* UNFINISHED */

#include "material_manager.hpp"

namespace VkRenderer {
	MaterialManager::MaterialManager(VulkanDevice& device)
		: device{ device }, buffer(device, 4, sizeof(Material)* MAX_MATERIAL_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		buffer.map();
	}

	MaterialManager::~MaterialManager()
	{}

	void MaterialManager::addMaterial(std::string name, Material& material)
	{
		if (materials.find(name) == materials.end()) {
			materials[name] = std::move(material);
			materialOrder.push_back(name);
		}
	}

	void MaterialManager::addTexture(std::string name, std::unique_ptr<VulkanTexture> texture)
	{
		if (textures.find(name) == textures.end()) {
			textures[name] = std::move(texture);
			textureOrder.push_back(name);
		}
	}

	void MaterialManager::updateGPUBuffer()
	{
		std::vector<Material> materialList;
		materialList.reserve(materials.size());

		for (const auto& pair : materials)
		{
			materialList.push_back(pair.second);
		}

		buffer.writeToBuffer(materialList.data());
	}

	VulkanTexture* MaterialManager::getTexture(const std::string& name) const
	{
		auto it = textures.find(name);

		if (it != textures.end())
			return it->second.get();
		
		return nullptr;
	}

	int MaterialManager::getTextureId(const std::string& name)
	{
		for (size_t i = 0; i < textureOrder.size(); ++i) {
			if (textureOrder[i] == name) {
				return i;
			}
		}

		return -1;
	}

	int MaterialManager::getMaterialId(const std::string& name)
	{
		for (size_t i = 0; i < materialOrder.size(); ++i) {
			if (materialOrder[i] == name) {
				return i;
			}
		}

		return -1;
	}
}