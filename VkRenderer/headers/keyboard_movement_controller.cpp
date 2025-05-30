#include "keyboard_movement_controller.hpp"

namespace VkRenderer {
	KeyboardMovementController::KeyboardMovementController(GLFWwindow* window)
	{
		glfwSetWindowUserPointer(window, this);
		glfwSetCursorPosCallback(window, mouseCallback);
	}

	void VkRenderer::KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float deltaTime, VulkanObject& object)
	{
		glm::vec3 rotate{ 0 };

		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS)
			rotate.y += 1.f;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
			rotate.y -= 1.f;
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
			rotate.x += 1.f;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
			rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			object.transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
		}

		object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
		object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

		float yaw = object.transform.rotation.y;

		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };

		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
			moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
			moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
			moveDir += rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
			moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
			moveDir += upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
			moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			object.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
		}
	}

	void KeyboardMovementController::rotateInPlaneXZ(float deltaTime, VulkanObject& object)
	{
		if (std::abs(deltaX) > std::numeric_limits<float>::epsilon() ||
			std::abs(deltaY) > std::numeric_limits<float>::epsilon()) {

			object.transform.rotation.y += lookSpeed * deltaTime * deltaX;
			object.transform.rotation.x += lookSpeed * deltaTime * -deltaY;

			object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
			object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

			deltaX = 0.0f;
			deltaY = 0.0f;
		}
	}

	void KeyboardMovementController::onCursorMove(GLFWwindow* window, double xpos, double ypos)
	{
		deltaX = static_cast<float>(xpos - lastX);
		deltaY = static_cast<float>(ypos - lastY);

		lastX = xpos;
		lastY = ypos;
	}
}