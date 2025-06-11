#include "VulkanObject.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace VkRenderer {
	glm::mat4 TransformComponent::mat4()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);

		return glm::mat4{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x, translation.y, translation.z, 1.0f} };
	}

	glm::mat3 TransformComponent::normalMatrix()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 invScale = 1.0f / scale;

		return glm::mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			},
		};
	}

	VulkanObject VulkanObject::makePointLight(float intensity, float radius, glm::vec3 color)
	{
		VulkanObject object = VulkanObject::create();
		object.color = color;
		object.transform.scale.x = radius;
		object.pointLight = std::make_unique<PointLightComponent>();
		object.pointLight->lightIntensity = intensity; 

		return object;
	}

	VulkanTexture* VulkanObject::createTexture(VulkanDevice& device, const char* path, VkFormat format)
	{
		VulkanTexture* texture = new VulkanTexture(device, format);

		if (path == nullptr) {
			uint32_t whitePixel = 0xFFFFFFFF;
			unsigned char* white = reinterpret_cast<unsigned char*>(&whitePixel);

			texture->width = 1;
			texture->height = 1;
			texture->load(white);

			return texture;
		}

		unsigned char* data = stbi_load(path, &texture->width, &texture->height, nullptr, STBI_rgb_alpha);

		texture->load(data);
		stbi_image_free(data);

		return texture;
	}
}