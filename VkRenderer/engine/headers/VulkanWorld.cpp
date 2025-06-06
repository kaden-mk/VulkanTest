#include "VulkanWorld.hpp"

namespace VkRenderer {
	VulkanWorld::VulkanWorld(VulkanWindow* window)
		: device(*window), renderer(*window, device)
	{
	}

	VulkanWorld::~VulkanWorld()
	{
	}

	void VulkanWorld::run()
	{

	}

	void VulkanWorld::addRenderSystem()
	{

	}

	VulkanDevice& VulkanWorld::getDevice()
	{
		return device;
	}

	VulkanRenderer& VulkanWorld::getRenderer()
	{
		return renderer;
	}

	float VulkanWorld::getDeltaTime()
	{
		return deltaTime;
	}
}