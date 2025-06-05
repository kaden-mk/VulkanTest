#pragma once

#include "VulkanModel.hpp"
#include "VulkanTexture.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace VkRenderer {
	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation{};

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;
	};

	class VulkanObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, VulkanObject>;

		static VulkanObject create() {
			static id_t currentId = 0;
			return VulkanObject{ currentId++ };
		}

		static VulkanObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));
		
		VulkanObject(const VulkanObject &) = delete;
		VulkanObject &operator=(const VulkanObject &) = delete;

		VulkanObject(VulkanObject &&) = default;
		VulkanObject &operator=(VulkanObject &&) = default;

		id_t getId() const { return id; }

		std::shared_ptr<VulkanModel> model{};
		uint32_t texture = NULL;
		glm::vec3 color{};
		TransformComponent transform{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;
	private:
		VulkanObject(id_t objId) : id(objId) {};

		id_t id;
	};
}