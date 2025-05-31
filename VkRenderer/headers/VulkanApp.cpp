#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "VulkanApp.hpp"

#include "RenderingSystem.hpp"
#include "VulkanCamera.hpp"
#include "keyboard_movement_controller.hpp"

namespace VkRenderer {
	VulkanApp::VulkanApp() {
		loadObjects();
	}

	VulkanApp::~VulkanApp() {}

	void VulkanApp::run()
	{
        glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		RenderingSystem renderSystem{ device, renderer.getSwapChainRenderPass() };
        VulkanCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));
        
        auto viewerObject = VulkanObject::create();

        KeyboardMovementController cameraController{window.getWindow()};

        std::chrono::high_resolution_clock::time_point lastFrameTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {
			glfwPollEvents();

            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrameTime).count();
            lastFrameTime = currentTime;

            cameraController.moveInPlaneXZ(window.getWindow(), deltaTime, viewerObject);
            cameraController.rotateInPlaneXZ(deltaTime, viewerObject);

            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

			if (auto commandBuffer = renderer.beginFrame()) {
				renderer.beginSwapChainRenderPass(commandBuffer);

				renderSystem.renderObjects(commandBuffer, objects, camera);
                window.updateTitle(1.f / deltaTime);

				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void VulkanApp::loadObjects()
	{
        std::shared_ptr<VulkanModel> model = VulkanModel::createModelFromFile(device, "VkRenderer/assets/smooth_vase.obj");

        auto cube = VulkanObject::create();
        cube.model = model;
        cube.transform.translation = { .0f, .0f, 2.5f };
        cube.transform.scale = { .5f, .5f, .5f };

        objects.push_back(std::move(cube));
	}
}