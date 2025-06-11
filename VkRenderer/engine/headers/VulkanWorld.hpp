#pragma once

#include "VulkanDescriptors.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanObject.h"
#include "VulkanFrameInfo.hpp"

#include "managers/material_manager.hpp"

#include <functional>

namespace VkRenderer {
	const uint32_t BINDING_STORAGE = 0;
	const uint32_t BINDING_SAMPLER = 1;
	const uint32_t BINDING_IMAGE = 2;
	const uint32_t BINDING_MATERIAL = 3;

	class VulkanWorld
	{
	public:
		using CreateRenderingSystems = std::function<void(VulkanDescriptorSetLayout*)>;
		using OnFrameUpdate = std::function<void()>;
		using OnUpdate = std::function<void(FrameInfo, GlobalUbo&)>;
		using OnRender = std::function<void(VkCommandBuffer commandBuffer, FrameInfo frameInfo)>;

		VulkanWorld(VulkanWindow &window);
		~VulkanWorld();

		VulkanWorld(const VulkanWorld&) = delete;
		VulkanWorld& operator=(const VulkanWorld&) = delete;

		void init();
		void parseImages(std::vector<VkDescriptorImageInfo> images);
		void addObject(VulkanObject::id_t, VulkanObject&& object);

		std::vector<VkDescriptorImageInfo> convertImages(const std::vector<VulkanTexture*>& textures);

		VulkanDevice& getDevice();
		VulkanRenderer& getRenderer();

		VulkanDescriptorPool* getGlobalPool() const { return globalPool.get(); }
		VulkanObject::Map& getObjects() { return worldObjects; }

		float getDeltaTime();

		void setCreateRenderingSystems(CreateRenderingSystems callback) { createRenderingSystemsCallback = callback; }
		void setOnFrameUpdate(OnFrameUpdate callback) { onFrameUpdateCallback = callback; }
		void setOnUpdate(OnUpdate callback) { onUpdateCallback = callback;}
		void setOnRender(OnRender callback) { onRenderCallback = callback; }

		float camera_nearDistance = 0.1f;
		float camera_farDistance = 10000.f;

		VulkanObject viewerObject = VulkanObject::create();
	protected:
		void createRenderingSystems(auto layout) { if (createRenderingSystemsCallback) createRenderingSystemsCallback(layout); }
		void onFrameUpdate() { if (onFrameUpdateCallback) onFrameUpdateCallback(); }
		void onUpdate(FrameInfo frameInfo, GlobalUbo &ubo) { if (onUpdateCallback) onUpdateCallback(frameInfo, ubo); }
		void onRender(VkCommandBuffer commandBuffer, FrameInfo frameInfo) { if (onRenderCallback) onRenderCallback(commandBuffer, frameInfo); }

	private:
		VulkanWindow& window;
		VulkanDevice device;
		VulkanRenderer renderer;

		CreateRenderingSystems createRenderingSystemsCallback;
		OnFrameUpdate onFrameUpdateCallback;
		OnUpdate onUpdateCallback;
		OnRender onRenderCallback;

		std::unique_ptr<VulkanDescriptorPool> globalPool{};
		VulkanObject::Map worldObjects;

		float deltaTime{ 0.0f };

		std::array<VkDescriptorBindingFlags, 4> bindingFlags{};
		std::vector<VkDescriptorImageInfo> imagesToWrite{};

		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> createBindings();
		VkDescriptorSetLayoutBindingFlagsCreateInfo createBindingFlags();

	public: // make it below device creation
		MaterialManager materialManager{ device };
	};
}