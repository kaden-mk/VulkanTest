#pragma once

#include "VulkanObject.h"
#include "VulkanWindow.hpp"

namespace VkRenderer {
	class MovementController {
	public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        MovementController(GLFWwindow* window);

        void moveInPlaneXZ(GLFWwindow* window, float deltaTime, VulkanObject& object);
        void rotateInPlaneXZ(float deltaTime, VulkanObject& object);
        void onCursorMove(GLFWwindow* window, double xpos, double ypos);

        KeyMappings keys{};
        float moveSpeed{ 3.f };
        float lookSpeed{ 0.6f };
    private:
        double lastX{ 0.0 };
        double lastY{ 0.0 };
        
        float deltaY{ 0.f };
        float deltaX{ 0.f };

        static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
            auto* self = static_cast<MovementController*>(glfwGetWindowUserPointer(window));

            if (self) self->onCursorMove(window, xpos, ypos);
        }
	};
}