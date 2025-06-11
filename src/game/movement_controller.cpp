#include "movement_controller.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace VkRenderer {
	MovementController::MovementController(GLFWwindow* window)
	{
		glfwSetWindowUserPointer(window, this);
	}

	void MovementController::moveInPlaneXZ(GLFWwindow* window, float deltaTime, VulkanObject& object)
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

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) 
			object.transform.rotation += sensitivity * deltaTime * glm::normalize(rotate);
		

		object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
		object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

		glm::vec3 rotation = object.transform.rotation;
		glm::mat4 rotationMatrix = glm::yawPitchRoll(rotation.y, rotation.x, rotation.z);

		glm::vec3 forwardDir = glm::vec3(rotationMatrix * glm::vec4(0.f, 0.f, -1.f, 0.f));
		glm::vec3 rightDir = glm::vec3(rotationMatrix * glm::vec4(1.f, 0.f, 0.f, 0.f));  
		glm::vec3 upDir = glm::vec3(rotationMatrix * glm::vec4(0.f, 1.f, 0.f, 0.f)); 

		glm::vec3 moveDir{ 0.f };

		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
			moveDir -= forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
			moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
			moveDir += rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
			moveDir -= rightDir;
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
			moveDir -= upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
			moveDir += upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) 
			object.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
	}

	void MovementController::rotateInPlaneXZ(float deltaTime, VulkanObject& object)
	{
		if (std::abs(deltaX) > std::numeric_limits<float>::epsilon() ||
			std::abs(deltaY) > std::numeric_limits<float>::epsilon()) {

			object.transform.rotation.y +=  deltaTime * deltaX;
			object.transform.rotation.x +=  deltaTime * -deltaY;

			object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
			object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

			deltaX = 0.0f;
			deltaY = 0.0f;
		}
	}

	void MovementController::resetCursor(double xpos, double ypos)
	{
		lastX = xpos;
		lastY = ypos;
		deltaX = 0.0f;
		deltaY = 0.0f;
	}

	void MovementController::onCursorMove(GLFWwindow* window, double xpos, double ypos)
	{
		deltaX = static_cast<float>(xpos - lastX) * sensitivity;
		deltaY = static_cast<float>(ypos - lastY) * sensitivity;

		lastX = xpos;
		lastY = ypos;
	}
}