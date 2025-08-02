#pragma once

#include "VulkanDevice.hpp"

namespace VkRenderer {
	class VulkanTexture
	{
		public:
			VulkanTexture(VulkanDevice& device, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB);
			~VulkanTexture();

			VulkanTexture(const VulkanTexture&) = delete;
			VulkanTexture& operator = (const VulkanTexture&) = delete;
			VulkanTexture(VulkanTexture&&) = delete;
			VulkanTexture& operator=(VulkanTexture&&) = delete;

			VkSampler getSampler() { return sampler; }
			VkImageView getImageView() { return imageView; }
			VkImageLayout getImageLayout() { return layout; }

			int width, height = 1;

			void load(void* data);
		private:
			void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
			void generateMipmaps();

			void createImageInfo(void* data);
			void createSamplerInfo();
			void createImageViewInfo();

			int mipLevels = 1;

			VulkanDevice& device;

			VkImageView imageView;
			VkDeviceMemory memory;
			VkSampler sampler;
			VkImage image;
			VkFormat format;
			VkImageLayout layout;
	};
}
