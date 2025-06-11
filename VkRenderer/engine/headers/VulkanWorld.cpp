#include "VulkanWorld.hpp"
#include "VulkanBuffer.hpp"

#include <iostream>

namespace VkRenderer {
	VulkanWorld::VulkanWorld(VulkanWindow& window)
		: window{ window }, device(window), renderer(window, device)
	{
		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, STORAGE_COUNT },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SAMPLER_COUNT },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, IMAGE_COUNT },
		};

		globalPool = VulkanDescriptorPool::Builder(device)
			.setPoolSizes(poolSizes)
			.setMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT * 2)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.build();

		// default material setup

		auto* defaultTex = VulkanObject::createTexture(device, nullptr);
		materialManager.addTexture("default", std::unique_ptr<VulkanTexture>(defaultTex));
		parseImages(convertImages({ defaultTex }));

		Material material{};
		material.albedoIndex = materialManager.getTextureId("default");
		material.normalIndex = materialManager.getTextureId("default");
		materialManager.addMaterial("default", material);
	}

	VulkanWorld::~VulkanWorld()
	{}

	void VulkanWorld::init()
	{
		// Create ubo buffers
		std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<VulkanBuffer>(
				device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			uboBuffers[i]->map();
		}

		// Set the descriptor (no idea what i should call this)
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings = createBindings();
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = createBindingFlags();

		auto globalSetLayout = VulkanDescriptorSetLayout::Builder(device)
			.setBindings(bindings)
			.setFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
			.setPNext(bindingFlagsInfo)
			.build();

		auto materialBufferInfo = materialManager.getDescriptorInfo();

		std::vector<VkDescriptorSet> globalDescriptorSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();

			VulkanDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(BINDING_STORAGE, &bufferInfo)
				.writeImageArray(BINDING_SAMPLER, imagesToWrite.data(), imagesToWrite.size())
				.writeBuffer(BINDING_MATERIAL, &materialBufferInfo)
				.build(globalDescriptorSets[i]);
		}

		// Rendering system initialization
		createRenderingSystems(globalSetLayout.get());

		// Camera creation
		VulkanCamera camera{};
		camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

		// Frames
		std::chrono::high_resolution_clock::time_point lastFrameTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {
			glfwPollEvents();

			auto currentTime = std::chrono::high_resolution_clock::now();
			deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrameTime).count();
			lastFrameTime = currentTime;

			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = renderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, camera_nearDistance, camera_farDistance);

			onFrameUpdate();

			if (auto commandBuffer = renderer.beginFrame()) {
				int frameIndex = renderer.getFrameIndex();

				FrameInfo frameInfo{
					frameIndex,
					deltaTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					worldObjects
				};

				// update
				GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseView();

				onUpdate(frameInfo, ubo);

				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// rendering

				renderer.beginSwapChainRenderPass(commandBuffer);

				onRender(commandBuffer, frameInfo);

				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		// Cleanup
		vkDeviceWaitIdle(device.device());
	}

	void VulkanWorld::parseImages(std::vector<VkDescriptorImageInfo> images)
	{
		// to allow for the function to be ran multiple times if needed
		for (auto& image : images) 
			imagesToWrite.push_back(image);
		
	}

	void VulkanWorld::addObject(VulkanObject::id_t id, VulkanObject&& object)
	{
		worldObjects.emplace(id, std::move(object));
	}

	// auto image converter for material manager
	std::vector<VkDescriptorImageInfo> VulkanWorld::convertImages(const std::vector<VulkanTexture*>& textures)
	{
		std::vector<VkDescriptorImageInfo> imagesToReturn;

		for (auto* texture : textures) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = texture->getImageLayout();
			imageInfo.imageView = texture->getImageView();
			imageInfo.sampler = texture->getSampler();
			imagesToReturn.push_back(imageInfo);
		}

		return imagesToReturn;
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

	// Utility functions
	
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> VulkanWorld::createBindings()
	{
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		bindings[BINDING_STORAGE] = {
			.binding = BINDING_STORAGE,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};

		bindings[BINDING_SAMPLER] = {
			.binding = BINDING_SAMPLER,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = SAMPLER_COUNT,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};

		bindings[BINDING_IMAGE] = {
			.binding = BINDING_IMAGE,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};

		bindings[BINDING_MATERIAL] = {
			.binding = BINDING_MATERIAL,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
		};

		return bindings;
	}

	VkDescriptorSetLayoutBindingFlagsCreateInfo VulkanWorld::createBindingFlags()
	{
		bindingFlags = {
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
		};

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();

		return bindingFlagsInfo;
	}
}