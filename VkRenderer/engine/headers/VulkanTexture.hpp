#pragma once

#include "VulkanDevice.hpp"

namespace VkRenderer {
	class VulkanTexture
	{
		public:
			VulkanTexture(VulkanDevice& device);
			~VulkanTexture();

			VulkanTexture(const VulkanTexture&) = delete;
			VulkanTexture& operator = (const VulkanTexture&) = delete;
			VulkanTexture(VulkanTexture&&) = delete;
			VulkanTexture& operator=(VulkanTexture&&) = delete;

			VkSampler getSampler() { return sampler; }
			VkImageView getImageView() { return imageView; }
			VkImageLayout getImageLayout() { return layout; }

			int width, height = 1;

			void load(unsigned char* data);
		private:
			void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
			void generateMipmaps();

			void createImageInfo(unsigned char* data);
			void createSamplerInfo();
			void createImageViewInfo();

			int mipLevels;

			VulkanDevice& device;

			VkImageView imageView;
			VkDeviceMemory memory;
			VkSampler sampler;
			VkImage image;
			VkFormat format;
			VkImageLayout layout;
	};
}
