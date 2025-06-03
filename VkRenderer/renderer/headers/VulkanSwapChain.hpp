#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <memory>

namespace VkRenderer {
	class VulkanSwapChain
	{
		public:
			static constexpr int MAX_FRAMES_IN_FLIGHT = 2; // the amount of framebuffers

			VulkanSwapChain(VulkanDevice & deviceRef, VkExtent2D windowExtent);
			VulkanSwapChain(VulkanDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<VulkanSwapChain> previous);
			~VulkanSwapChain();

			VulkanSwapChain(const VulkanSwapChain&) = delete;
			VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

			VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
			VkRenderPass getRenderPass() const { return renderPass; }
			VkImageView getImageView(int index) { return swapChainImageViews[index]; }
			size_t imageCount() { return swapChainImages.size(); }
			VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
			VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
			uint32_t width() const { return swapChainExtent.width; }
			uint32_t height() const { return swapChainExtent.height; }
			
			float extentAspectRatio() const {
				return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
			}
			VkFormat findDepthFormat();

			VkResult acquireNextImage(uint32_t* imageIndex);
			VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

			bool compareSwapFormats(const VulkanSwapChain& swapChain) const {
				return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
			}
		private:
			void init();
			void createSwapChain();
			void createImageViews();
			void createDepthResources();
			void createRenderPass();
			void createFramebuffers();
			void createSyncObjects();

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

			VkFormat swapChainImageFormat;
			VkFormat swapChainDepthFormat;
			VkExtent2D swapChainExtent;

			std::vector<VkFramebuffer> swapChainFramebuffers;
			VkRenderPass renderPass;

			std::vector<VkImage> depthImages;
			std::vector<VkDeviceMemory> depthImageMemorys;
			std::vector<VkImageView> depthImageViews;
			std::vector<VkImage> swapChainImages;
			std::vector<VkImageView> swapChainImageViews;

			VulkanDevice &device;
			VkExtent2D windowExtent;

			VkSwapchainKHR swapChain;
			std::shared_ptr<VulkanSwapChain> oldSwapChain;

			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
			size_t currentFrame = 0;
	};
}