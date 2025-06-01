#pragma once

#include "VulkanModel.hpp"

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

	class VulkanObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, VulkanObject>;

		static VulkanObject create() {
			static id_t currentId = 0;
			return VulkanObject{ currentId++ };
		}
		
		VulkanObject(const VulkanObject &) = delete;
		VulkanObject &operator=(const VulkanObject &) = delete;

		VulkanObject(VulkanObject &&) = default;
		VulkanObject &operator=(VulkanObject &&) = default;

		id_t getId() const { return id; }

		std::shared_ptr<VulkanModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};
	private:
		VulkanObject(id_t objId) : id(objId) {};

		id_t id;
	};
}