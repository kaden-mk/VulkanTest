#pragma once

#include "VulkanDescriptors.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanObject.h"

namespace VkRenderer {
	class VulkanWorld
	{
	public:
		VulkanWorld(VulkanWindow* window);
		~VulkanWorld();

		VulkanWorld(const VulkanWorld&) = delete;
		VulkanWorld& operator=(const VulkanWorld&) = delete;

		void run();
		void addRenderSystem();

		VulkanDevice& getDevice();
		VulkanRenderer& getRenderer();

		float getDeltaTime();
	private:
		std::unique_ptr<VulkanDescriptorPool> globalPool{};

		VulkanDevice device;
		VulkanRenderer renderer;

		VulkanObject viewerObject = VulkanObject::create();

		float deltaTime;
	};
}