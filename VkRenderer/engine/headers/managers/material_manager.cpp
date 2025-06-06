/* UNFINISHED */

#include "material_manager.hpp"

namespace VkRenderer {
	MaterialManager::MaterialManager(VulkanDevice& device)
		: device{device}, buffer(device, 4, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	{
	}

	MaterialManager::~MaterialManager()
	{

	}

	void MaterialManager::addMaterial(Material& material)
	{

	}

	void MaterialManager::updateGPUBuffer()
	{

	}
}