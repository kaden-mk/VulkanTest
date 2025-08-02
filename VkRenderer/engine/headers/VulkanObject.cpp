#include "VulkanObject.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include <algorithm>
#include <string>
#include <filesystem>

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
		glm::mat4 model =
			glm::translate(glm::mat4(1.0f), translation) *
			glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
			glm::scale(glm::mat4(1.0f), scale);

		return glm::transpose(glm::inverse(glm::mat3(model)));
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
		else if (path == "no_texture_normal") {
			const int width = 256;
			const int height = 256;

			std::vector<uint8_t> pixelData(width * height * 4);

			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					int i = y * width + x;
					pixelData[i * 4 + 0] = 128;
					pixelData[i * 4 + 1] = 128;
					pixelData[i * 4 + 2] = 255;
					pixelData[i * 4 + 3] = 255; 
				}
			}

			unsigned char* normalData = reinterpret_cast<unsigned char*>(pixelData.data());

			texture->width = width;
			texture->height = height;
			texture->load(normalData);

			return texture;
		}

		std::string extension = std::filesystem::path(path).extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

		if (extension == ".hdr") {
			stbi_set_flip_vertically_on_load(true);
			float* hdrData = stbi_loadf(path, &texture->width, &texture->height, nullptr, 4);
			if (!hdrData) {
				std::cerr << "Failed to load HDR image: " << path << std::endl;
				delete texture;
				return nullptr;
			}

			texture->load(hdrData);
			stbi_image_free(hdrData);
		}
		else {
			unsigned char* ldrData = stbi_load(path, &texture->width, &texture->height, nullptr, STBI_rgb_alpha);
			if (!ldrData) {
				std::cerr << "Failed to load image: " << path << std::endl;
				delete texture;
				return nullptr;
			}

			texture->load(ldrData);
			stbi_image_free(ldrData);
		}

		return texture;
	}
}