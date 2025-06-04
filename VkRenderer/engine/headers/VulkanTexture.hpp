#pragma once

#include "VulkanDevice.hpp"

#include "stb_image.h"

namespace VkRenderer {
	class VulkanTexture
	{
		public:
			VulkanTexture(VulkanDevice& device, const char* path);
			~VulkanTexture();

			VulkanTexture(const VulkanTexture&) = delete;
			VulkanTexture& operator = (const VulkanTexture&) = delete;
			VulkanTexture(VulkanTexture&&) = delete;
			VulkanTexture& operator=(VulkanTexture&&) = delete;

			VkSampler getSampler() { return sampler; }
			VkImageView getImageView() { return imageView; }
			VkImageLayout getImageLayout() { return layout; }
		private:
			void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
			void generateMipmaps();

			void createImageInfo(stbi_uc* data);
			void createSamplerInfo();
			void createImageViewInfo();

			int width, height, mipLevels;

			VulkanDevice& device;

			VkImageView imageView;
			VkDeviceMemory memory;
			VkSampler sampler;
			VkImage image;
			VkFormat format;
			VkImageLayout layout;
	};
}
