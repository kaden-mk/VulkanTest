#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanUtils.hpp"

#include <stdexcept>
#include <utility>

namespace VkRenderer {
	VulkanTexture::VulkanTexture(VulkanDevice& device) : device{ device } 
	{
		format = VK_FORMAT_R8G8B8A8_SRGB;
		layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	
	VulkanTexture::~VulkanTexture() {
		vkDestroySampler(device.device(), sampler, nullptr);
		vkDestroyImageView(device.device(), imageView, nullptr);
		vkDestroyImage(device.device(), image, nullptr);
		vkFreeMemory(device.device(), memory, nullptr);
	}

	void VulkanTexture::load(unsigned char* data)
	{
		createImageInfo(data);
		createSamplerInfo();
		createImageViewInfo();
	}

	void VulkanTexture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		struct Transition {
			VkAccessFlags srcAccessMask;
			VkAccessFlags dstAccessMask;
			VkPipelineStageFlags srcStage;
			VkPipelineStageFlags dstStage;
		};

		struct PairHash {
			std::size_t operator()(const std::pair<VkImageLayout, VkImageLayout>& pair) const {
				std::size_t seed = 0;
				hashCombine(seed, pair.first, pair.second);
				return seed;
			}
		};

		std::unordered_map<std::pair<VkImageLayout, VkImageLayout>, Transition, PairHash> transitions = {
			{{VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
				{0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT}},

			{{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				{VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT}}
		};

		auto it = transitions.find({ oldLayout, newLayout });
		if (it != transitions.end()) {
			auto& second = it->second;

			barrier.srcAccessMask = second.srcAccessMask;
			barrier.dstAccessMask = second.dstAccessMask;
			srcStage = second.srcStage;
			dstStage = second.dstStage;
		}
		else {
			throw std::runtime_error("Unsupported layout transition!");
		}
	
		vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		device.endSingleTimeCommands(commandBuffer);
	}

	void VulkanTexture::generateMipmaps()
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(device.physical(), format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			throw std::runtime_error("texture format doesn't support linear blitting");

		VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
			
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		device.endSingleTimeCommands(commandBuffer);
	}

	void VulkanTexture::createImageInfo(unsigned char* data)
	{
		mipLevels = std::floor(std::log2(std::max(width, height))) + 1;

		VulkanBuffer stagingBuffer{
			device,
			4,
			static_cast<uint32_t>(width * height),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(data);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
		transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		device.copyBufferToImage(stagingBuffer.getBuffer(), image, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1);
		generateMipmaps();
	}

	void VulkanTexture::createSamplerInfo()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = device.properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mipLevels);

		if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create image sampler");
	}

	void VulkanTexture::createImageViewInfo()
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture image view");
	}
}