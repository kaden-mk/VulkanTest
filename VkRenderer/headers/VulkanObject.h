#pragma once

#include "VulkanModel.hpp"

#include <memory>

namespace VkRenderer {
	struct Transform2DComponent {
		glm::vec2 translation{};
		glm::vec2 scale{1.f, 1.f};
		float rotation;

		glm::mat2 mat2() const {
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			
			glm::mat2 rotationMat{ {c, s}, {-s, c} };
			glm::mat2 scaleMat{ { scale.x, .0f }, {.0f, scale.y} };
			return rotationMat * scaleMat;
		}
	};

	class VulkanObject
	{
	public:
		using id_t = unsigned int;

		static VulkanObject create() {
			static id_t currentId = 0;
			return VulkanObject{ currentId++ };
		}
		
		VulkanObject(const VulkanObject&) = delete;
		VulkanObject& operator=(const VulkanObject&) = delete;
		VulkanObject(VulkanObject&&) = default;
		VulkanObject& operator=(VulkanObject&&) = default;

		id_t getId() const { return id; }

		std::shared_ptr<VulkanModel> model{};
		glm::vec3 color{};
		Transform2DComponent transform2D{};
	private:
		VulkanObject(id_t objId) : id(objId) {};

		id_t id;
	};
}