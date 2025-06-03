#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "main_game.hpp"
#include "movement_controller.hpp"

#include "systems/RenderingSystem.hpp"
#include "systems/PointLightSystem.hpp"

#include "VulkanCamera.hpp"
#include "VulkanBuffer.hpp"

namespace VkRenderer {
    Game::Game() {
        globalPool = VulkanDescriptorPool::Builder(device)
            .setMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();

		loadObjects();
	}

    Game::~Game() {}

	void Game::run()
	{
        std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<VulkanBuffer>(
                device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );

            uboBuffers[i]->map();
        }

        auto globalSetLayout = VulkanDescriptorSetLayout::Builder(device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets{VulkanSwapChain::MAX_FRAMES_IN_FLIGHT};
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        RenderingSystem renderSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        PointLightSystem pointLightSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

        VulkanCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));
        
        auto viewerObject = VulkanObject::create();
        viewerObject.transform.translation.z = -2.5f;

        MovementController cameraController{window.getWindow()};

        std::chrono::high_resolution_clock::time_point lastFrameTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {
			glfwPollEvents();

            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrameTime).count();
            lastFrameTime = currentTime;

            window.updateTitle(1.f / deltaTime);

            cameraController.moveInPlaneXZ(window.getWindow(), deltaTime, viewerObject);
            cameraController.rotateInPlaneXZ(deltaTime, viewerObject);

            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10000.f);

			if (auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();

                FrameInfo frameInfo{
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    objects
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();

                pointLightSystem.update(frameInfo, ubo);
              
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // rendering

				renderer.beginSwapChainRenderPass(commandBuffer);

				renderSystem.renderObjects(frameInfo);
                pointLightSystem.render(frameInfo);

				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

	void Game::loadObjects()
	{
        std::shared_ptr<VulkanModel> model = VulkanModel::createModelFromFile(device, "assets/models/smooth_vase.obj");
        auto flatVase = VulkanObject::create();
        flatVase.model = model;
        flatVase.transform.translation = { -.5f, .5f, 0.f };
        flatVase.transform.scale = { 1.f, 1.f, 1.f };

        model = VulkanModel::createModelFromFile(device, "assets/models/smooth_vase.obj");
        auto smoothVase = VulkanObject::create();
        smoothVase.model = model;
        smoothVase.transform.translation = { .5f, .5f, 0.f };
        smoothVase.transform.scale = { 1.f, 1.f, 1.f };

        model = VulkanModel::createModelFromFile(device, "assets/models/quad.obj");
        auto floor = VulkanObject::create();
        floor.model = model;
        floor.transform.translation = { 0.f, .5f, 0.f };
        floor.transform.scale = { 3.f, 1.f, 3.f };

        std::vector<glm::vec3> lightColors{
             {1.f, .1f, .1f},
             {.1f, .1f, 1.f},
             {.1f, 1.f, .1f},
             {1.f, 1.f, .1f},
             {.1f, 1.f, 1.f},
             {1.f, 1.f, 1.f}
        };

        for (int i = 0; i < lightColors.size(); i++) {
            glm::vec3 color = lightColors[i];

            auto pointLight = VulkanObject::makePointLight(0.2f);
            pointLight.color = color;

            auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f, -1.f, 0.f });

            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
                
            objects.emplace(pointLight.getId(), std::move(pointLight));
        }

        objects.emplace(floor.getId(), std::move(floor));
        objects.emplace(flatVase.getId(), std::move(flatVase));
        objects.emplace(smoothVase.getId(), std::move(smoothVase));
  	}
}