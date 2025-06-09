#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "main_game.hpp"

#include "systems/RenderingSystem.hpp"
#include "systems/PointLightSystem.hpp"

#include "VulkanCamera.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanUtils.hpp"

#include <thread>
#include <chrono>

namespace VkRenderer {
    Game::Game() : materialBuffer(device, 4, sizeof(Material) * MAX_MATERIAL_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, STORAGE_COUNT },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SAMPLER_COUNT },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, IMAGE_COUNT },
        };

        globalPool = VulkanDescriptorPool::Builder(device)
            .setPoolSizes(poolSizes)
            .setMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT * 2)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
            .build();

        materialBuffer.map();

        /* VulkanWorld */
        initImGui();
        loadTextures();
		loadObjects();
        /* VulkanWorld */
	}

    Game::~Game() {
        /* VulkanWorld */
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        /* VulkanWorld */
    }

	void Game::run()
	{
        std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<VulkanBuffer>(
                device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );

            uboBuffers[i]->map();
        }

        /* VulkanWorld (queryImages) */
        std::vector<VkDescriptorImageInfo> imagesToWrite{};

        for (auto& texturePtr : textures) {
            VulkanTexture& texture = *texturePtr;
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = texture.getImageLayout();
            imageInfo.imageView = texture.getImageView();
            imageInfo.sampler = texture.getSampler();
            imagesToWrite.push_back(imageInfo);  
        }
        /* VulkanWorld */

        const uint32_t BINDING_STORAGE = 0;
        const uint32_t BINDING_SAMPLER = 1;
        const uint32_t BINDING_IMAGE = 2;
        const uint32_t BINDING_MATERIAL = 3;

        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        bindings[BINDING_STORAGE] = {
            .binding = BINDING_STORAGE,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        };

        bindings[BINDING_SAMPLER] = {
            .binding = BINDING_SAMPLER,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = SAMPLER_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL, 
        };

        bindings[BINDING_IMAGE] = {
            .binding = BINDING_IMAGE,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        };

        bindings[BINDING_MATERIAL] = {
            .binding = BINDING_MATERIAL,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        };

        VkDescriptorBindingFlags bindingFlags[4] = {
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        };

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = {};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.bindingCount = 4;
        bindingFlagsInfo.pBindingFlags = bindingFlags;

        auto globalSetLayout = VulkanDescriptorSetLayout::Builder(device)
            .setBindings(bindings)
            .setFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
            .setPNext(bindingFlagsInfo)
            .build();

        auto materialBufferInfo = materialBuffer.descriptorInfo();

        std::vector<VkDescriptorSet> globalDescriptorSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < VulkanSwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();

            VulkanDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(BINDING_STORAGE, &bufferInfo)
                .writeImageArray(BINDING_SAMPLER, imagesToWrite.data(), imagesToWrite.size())
                .writeBuffer(BINDING_MATERIAL, &materialBufferInfo)
                .build(globalDescriptorSets[i]);
        }

        glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        /* VulkanWorld */
        RenderingSystem renderSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        PointLightSystem pointLightSystem{ device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        /* VulkanWorld */

        VulkanCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));
        
        viewerObject.transform.translation.z = -2.5f;

        std::chrono::high_resolution_clock::time_point lastFrameTime = std::chrono::high_resolution_clock::now();

		while (!window.shouldClose()) {
			glfwPollEvents();

            // imgui handling

            /* VulkanWorld */
            VkExtent2D extent = window.getExtent();

            if (extent.width == 0 || extent.height == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            if (isToggled(GLFW_KEY_F1)) {
                showImGui = !showImGui;
                glfwSetInputMode(window.getWindow(), GLFW_CURSOR, showImGui == false ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

                if (showImGui == false) {
                    double x, y;
                    glfwGetCursorPos(window.getWindow(), &x, &y);
                    cameraController.resetCursor(x, y);
                }
            }

            if (window.wasWindowResized()) {
                ImGuiIO& io = ImGui::GetIO();
                io.DisplaySize = ImVec2((float)extent.width, (float)extent.height);
            }
            /* VulkanWorld */

            // imgui handling end

            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastFrameTime).count();
            lastFrameTime = currentTime;

            window.updateTitle(1.f / deltaTime);

            /* VulkanWorld */
            // whatever im hardcoding it
            if (showImGui == false) {
                double xPos;
                double yPos;

                glfwGetCursorPos(window.getWindow(), &xPos, &yPos);

                cameraController.onCursorMove(window.getWindow(), xPos, yPos);
            }

            cameraController.moveInPlaneXZ(window.getWindow(), deltaTime, viewerObject);
            cameraController.rotateInPlaneXZ(deltaTime, viewerObject);
            /* VulkanWorld */

            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, nearDistance, farDistance);

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

                /* VulkanWorld */
                pointLightSystem.update(frameInfo, ubo);
                /* VulkanWorld */

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // rendering

				renderer.beginSwapChainRenderPass(commandBuffer);

                /* VulkanWorld */
				renderSystem.renderObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                runImGui(commandBuffer);
                /* VulkanWorld */

				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(device.device());
	}

    void Game::initImGui()
    {
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_DockingEnable;
        
        ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = device.getInstance();
        info.DescriptorPool = globalPool.get()->getDescriptorPool();
        info.RenderPass = renderer.getSwapChainRenderPass();
        info.Device = device.device();
        info.PhysicalDevice = device.physical();
        info.Queue = device.graphicsQueue();
        info.QueueFamily = device.getGraphicsQueueFamily();
        info.MinImageCount = VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
        info.ImageCount = renderer.getSwapChainImageCount();
        info.Subpass = 0;
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&info);
    }

    void Game::runImGui(VkCommandBuffer commandBuffer)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int selectedObjectId = -1;

        if (showImGui == true) {
            ImGui::Begin("Object Manager");

            if (ImGui::Button("Create") == true) {
                std::shared_ptr<VulkanModel> model = VulkanModel::createModelFromFile(device, "assets/models/cube.obj");
                auto object = VulkanObject::create();
                object.model = model;
                objects.emplace(object.getId(), std::move(object));
            }

            if (ImGui::IsMouseDown(0)) {
            }

            if (selectedObjectId != -1) {
                ImGui::Text("Selected Object: %s", selectedObjectId);
            }
            else {
                ImGui::Text("No object selected");
            }

            for (auto& [id, object] : objects) {
                std::string name = "Object - " + std::to_string(id);

                ImGui::Begin(name.c_str());
                ImGui::DragFloat3("Position", glm::value_ptr(object.transform.translation), 0.1f);
                ImGui::DragFloat3("Rotation", glm::value_ptr(object.transform.rotation), 0.1f);
                ImGui::DragFloat3("Scale", glm::value_ptr(object.transform.scale), 0.1f);

                // what type of tom foolery is this
                if (object.pointLight == nullptr) {
                    int materialId = static_cast<int>(object.material);
                    if (ImGui::InputInt("MaterialId", &materialId))
                        object.material = static_cast<uint32_t>(materialId);
                }
                 
                if (object.pointLight != nullptr) {
                    ImGui::DragFloat("Light Intensity", &object.pointLight.get()->lightIntensity, 0.1f);
                    ImGui::DragFloat("Light Radius", &object.transform.scale.x, 0.1f);
                    ImGui::ColorEdit3("Light Color", glm::value_ptr(object.color));
                }
                
                ImGui::End();
            }

            ImGui::End();

            ImGui::Begin("Camera Manager");

            ImGui::DragFloat("Speed", &cameraController.moveSpeed, 0.1f);
            ImGui::DragFloat("Near Render Distance", &nearDistance, 0.1f);
            ImGui::DragFloat("Far Render Distance", &farDistance, 0.1f);
            
            if (ImGui::Button("Reset Camera Position") == true)
                viewerObject.transform.translation = { 0.f, 0.f, 0.f };

            ImGui::End();
        };
        
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void Game::loadObjects()
	{
        std::shared_ptr<VulkanModel> model = VulkanModel::createModelFromFile(device, "assets/models/quad.obj");
        auto floor = VulkanObject::create();
        floor.model = model;
        floor.material = 1;
        floor.transform.translation = { 0.f, .5f, 0.f };
        floor.transform.scale = { 3.f, 1.f, 3.f };

        VulkanObject pointLight = VulkanObject::makePointLight();
        pointLight.color = { 1.f, 1.f, 1.f };
        pointLight.transform.translation = { 0.f, -5.f, 0.f };

        objects.emplace(pointLight.getId(), std::move(pointLight));
        objects.emplace(floor.getId(), std::move(floor));
  	}

    // todo: make some sort of like texture manager or something instead
    void Game::loadTextures()
    {
        {
            Material material{};

            textures.push_back(std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, nullptr)));

            material.albedoIndex = 0;
            material.normalIndex = 0;

            materials.push_back(material);
        }
        {
            Material material{};

            textures.push_back(std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, "assets/textures/brick/color.jpg")));
            textures.push_back(std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, "assets/textures/brick/normal.png")));

            material.albedoIndex = 1;
            material.normalIndex = 2;
            materials.push_back(material);
        }
        {
            Material material{};

            textures.push_back(std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, "assets/textures/wood/color.jpg")));
            textures.push_back(std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, "assets/textures/wood/normal.png")));

            material.albedoIndex = 3;
            material.normalIndex = 4;
            materials.push_back(material);
        }

        materialBuffer.writeToBuffer(materials.data());
    }

    bool Game::isToggled(auto key)
    {
        static bool wasPressed = false;
        bool isPressedNow = glfwGetKey(window.getWindow(), key) == GLFW_PRESS;

        if (isPressedNow && !wasPressed) {
            wasPressed = true;
            return true;
        }

        if (!isPressedNow) {
            wasPressed = false;
        }

        return false;
    }
}