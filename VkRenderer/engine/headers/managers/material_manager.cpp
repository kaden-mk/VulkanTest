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
			materialOrder.push_back(material);
			size_t index = materialOrder.size() - 1;
			materials[name] = index;
		}
	}

	void MaterialManager::addTexture(std::string name, std::unique_ptr<VulkanTexture> texture)
	{
		if (textures.find(name) == textures.end()) {
			textureOrder.push_back(std::move(texture));
			size_t index = textureOrder.size() - 1;
			textures[name] = index;

			std::cout << "Texture '" << name << "' index = " << textureOrder.size() - 1 << "\n";
		}
	}

	void MaterialManager::updateGPUBuffer()
	{
		buffer.writeToBuffer(materialOrder.data());
	}

	VulkanTexture* MaterialManager::getTexture(const std::string& name) const
	{
		auto it = textures.find(name);

		if (it != textures.end()) {
			size_t Index = it->second;

			if (Index < textureOrder.size()) 
				return textureOrder[Index].get();
			
		}

		return nullptr;
	}

	int MaterialManager::getTextureId(const std::string& name)
	{
		auto it = textures.find(name);

		if (it != textures.end())
			return it->second;

		return -1;
	}

	int MaterialManager::getMaterialId(const std::string& name)
	{
		auto it = materials.find(name);

		if (it != materials.end()) 
			return it->second;
		
		return -1;
	}
}