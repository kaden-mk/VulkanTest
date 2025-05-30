#pragma once

#include "VulkanDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace VkRenderer {
	class VulkanModel
	{
		public:
			struct Vertex {
				glm::vec3 position;
				glm::vec3 color;

				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			};

			VulkanModel(VulkanDevice& device, const std::vector<Vertex> &vertices);
			~VulkanModel();

			VulkanModel(const VulkanModel&) = delete;
			VulkanModel& operator=(const VulkanModel&) = delete;

			void bind(VkCommandBuffer commandBuffer);
			void draw(VkCommandBuffer commandBuffer);
		private:
			void createVertexBuffers(const std::vector<Vertex>& vertices);


			VulkanDevice& device;
			VkBuffer vertexBuffer;
			VkDeviceMemory vertexBufferMemory;

			uint32_t vertexCount;
	};
}