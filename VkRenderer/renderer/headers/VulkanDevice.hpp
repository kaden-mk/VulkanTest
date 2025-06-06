#pragma once

#include "VulkanWindow.hpp"

#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>
#include <optional>

namespace VkRenderer {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool graphicsFamilyHasValue = false;
		bool presentFamilyHasValue = false;
		bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
	};

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
	};

	class VulkanDevice
	{
		public:
			#ifdef NDEBUG
			const bool enableValidationLayers = false;
			#else
			const bool enableValidationLayers = true;
			#endif
			
			VulkanDevice(VulkanWindow &window);
			~VulkanDevice();

			// Not copyable or movable
			VulkanDevice(const VulkanDevice&) = delete;
			VulkanDevice operator=(const VulkanDevice&) = delete;
			VulkanDevice(VulkanDevice&&) = delete;
			VulkanDevice& operator=(VulkanDevice&&) = delete;

			VkCommandPool getCommandPool() { return commandPool; }
			VkDevice device() { return _device; }
			VkPhysicalDevice physical() { return physicalDevice; }
			VkSurfaceKHR surface() { return _surface; }
			VkQueue graphicsQueue() { return _graphicsQueue; }
			VkQueue presentQueue() { return _presentQueue; }
			VkInstance getInstance() { return instance; }
			uint32_t getGraphicsQueueFamily() { return graphicsQueueFamily; }

			SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
			uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
			QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
			VkFormat findSupportedFormat(
				const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

			// Buffer Helper Functions
			void createBuffer(
				VkDeviceSize size,
				VkBufferUsageFlags usage,
				VkMemoryPropertyFlags properties,
				VkBuffer& buffer,
				VkDeviceMemory& bufferMemory);
			VkCommandBuffer beginSingleTimeCommands();
			void endSingleTimeCommands(VkCommandBuffer commandBuffer);
			void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
			void copyBufferToImage(
				VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

			void createImageWithInfo(
				const VkImageCreateInfo& imageInfo,
				VkMemoryPropertyFlags properties,
				VkImage& image,
				VkDeviceMemory& imageMemory);

			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceMemoryProperties memoryProperties;
		private:
			void createInstance();
			void setupDebugMessenger();
			void createSurface();
			void pickPhysicalDevice();
			void createLogicalDevice();
			void createCommandPool();

			// helper functions
			bool isDeviceSuitable(VkPhysicalDevice device);
			std::vector<const char*> getRequiredExtensions();
			bool checkValidationLayerSupport();
			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
			void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
			void hasGflwRequiredInstanceExtensions();
			bool checkDeviceExtensionSupport(VkPhysicalDevice device);
			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

			VkInstance instance;
			VkDebugUtilsMessengerEXT debugMessenger;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VulkanWindow& window;
			VkCommandPool commandPool;

			VkDevice _device;
			VkSurfaceKHR _surface;
			VkQueue _graphicsQueue;
			VkQueue _presentQueue;

			uint32_t graphicsQueueFamily;
	};
}