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

	// update these later
	void MaterialManager::addMaterial(Material& material)
	{
		materials.push_back(material);
	}

	void MaterialManager::updateGPUBuffer()
	{
		buffer.writeToBuffer(materials.data());
	}
}