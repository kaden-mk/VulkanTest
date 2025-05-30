#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "VulkanApp.hpp"

#include "RenderingSystem.hpp"

namespace VkRenderer {
	VulkanApp::VulkanApp() {
		loadObjects();
	}

	VulkanApp::~VulkanApp() {}

	void VulkanApp::run()
	{
		RenderingSystem renderSystem{ device, renderer.getSwapChainRenderPass() };

		while (!window.shouldClose()) {
			glfwPollEvents();

			if (auto commandBuffer = renderer.beginFrame()) {
				renderer.beginSwapChainRenderPass(commandBuffer);

				renderSystem.renderObjects(commandBuffer, objects);

				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void VulkanApp::loadObjects()
	{
		std::vector<VulkanModel::Vertex> vertices
		{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto model = std::make_shared<VulkanModel>(device, vertices);
		
		auto triangle = VulkanObject::create();
		triangle.model = model;
		triangle.color = { .1f, .8f, .1f };
		triangle.transform2D.translation.x = .2f;
		triangle.transform2D.scale = { 2.f, .5f };
		triangle.transform2D.rotation = .25f * glm::two_pi<float>();

		objects.push_back(std::move(triangle));
	}
}