#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "main_game.hpp"

#include "VulkanCamera.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanUtils.hpp"

#include <thread>
#include <chrono>

namespace VkRenderer {
    Game::Game()
    {
        initImGui();
        loadTextures();
		loadObjects();
	}

    Game::~Game()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

	void Game::run()
	{
        // disable the cursor
        glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        world.setCreateRenderingSystems([this](auto layout) {
            VulkanDevice& device = world.getDevice();
            VkRenderPass renderPass = world.getRenderer().getSwapChainRenderPass();

            this->renderingSystem = std::make_unique<RenderingSystem>(device, renderPass, layout->getDescriptorSetLayout());
            this->pointLightSystem = std::make_unique<PointLightSystem>(device, renderPass, layout->getDescriptorSetLayout());
        });

        world.setOnFrameUpdate([this]() {
            VkExtent2D extent = window.getExtent();

            if (extent.width == 0 || extent.height == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return;
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

            window.updateTitle(1.f / world.getDeltaTime());

            if (showImGui == false) {
                double xPos;
                double yPos;

                glfwGetCursorPos(window.getWindow(), &xPos, &yPos);

                cameraController.onCursorMove(window.getWindow(), xPos, yPos);
            }

            cameraController.moveInPlaneXZ(window.getWindow(), world.getDeltaTime(), world.viewerObject);
            cameraController.rotateInPlaneXZ(world.getDeltaTime(), world.viewerObject);
        });

        world.setOnUpdate([this](FrameInfo frameInfo, GlobalUbo& ubo) {
            pointLightSystem->update(frameInfo, ubo);
        });

        world.setOnRender([this](VkCommandBuffer commandBuffer, FrameInfo frameInfo) {
            renderingSystem->renderObjects(frameInfo);
            pointLightSystem->render(frameInfo);

            runImGui(commandBuffer);
        });

        world.init();
	}

    void Game::initImGui()
    {
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_DockingEnable;
        
        ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);

        VulkanDevice& device = world.getDevice();
        VulkanRenderer& renderer = world.getRenderer();

        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = device.getInstance();
        info.DescriptorPool = world.getGlobalPool()->getDescriptorPool();
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
                /*std::shared_ptr<VulkanModel> model = VulkanModel::createModelFromFile(device, "assets/models/cube.obj");
                auto object = VulkanObject::create();
                object.model = model;
                objects.emplace(object.getId(), std::move(object));*/
            }

            // for selecting objects (soon)
            if (ImGui::IsMouseDown(0)) {
            }

            if (selectedObjectId != -1) {
                ImGui::Text("Selected Object: %s", selectedObjectId);
            }
            else {
                ImGui::Text("No object selected");
            }

            for (auto& [id, object] : world.getObjects()) {
                std::string name = "Object - " + std::to_string(id);

                ImGui::Begin(name.c_str());
                ImGui::DragFloat3("Position", glm::value_ptr(object.transform.translation), 0.1f);
                ImGui::DragFloat3("Rotation", glm::value_ptr(object.transform.rotation), 0.1f);
                ImGui::DragFloat3("Scale", glm::value_ptr(object.transform.scale), 0.1f);

                // what type of tom foolery is this
                if (object.pointLight == nullptr) {
                    const auto& materials = world.materialManager.getMaterials();
                    std::vector<const char*> materialNames;

                    for (const std::string& name : materials)
                        materialNames.push_back(name.c_str());

                    int currentMaterialIndex = object.material;

                    if (ImGui::Combo("Material", &currentMaterialIndex, materialNames.data(), materialNames.size())) 
                        object.material = currentMaterialIndex;
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
            ImGui::DragFloat("Near Render Distance", &world.camera_nearDistance, 0.1f);
            ImGui::DragFloat("Far Render Distance", &world.camera_farDistance, 0.1f);
            
            if (ImGui::Button("Reset Camera Position") == true)
                world.viewerObject.transform.translation = { 0.f, 0.f, 0.f };

            ImGui::End();
        };
        
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void Game::loadObjects()
	{
        VulkanDevice& device = world.getDevice();
        MaterialManager& materialManager = world.materialManager;

        std::shared_ptr<VulkanModel> model = VulkanModel::createModelFromFile(device, "assets/models/quad.obj");
        auto floor = VulkanObject::create();
        floor.model = model;
        floor.material = materialManager.getMaterialId("brick");
        floor.transform.translation = { 0.f, .5f, 0.f };
        floor.transform.scale = { 3.f, 1.f, 3.f };

        VulkanObject pointLight = VulkanObject::makePointLight();
        pointLight.color = { 1.f, 1.f, 1.f };
        pointLight.transform.translation = { 0.f, -5.f, 0.f };

        world.addObject(floor.getId(), std::move(floor));
        world.addObject(pointLight.getId(), std::move(pointLight));
  	}

    void Game::loadTextures()
    {
        /* This NEEDS to be cleaned up HEAVILY. This should be done by the game looping through assets and loading them automatically. */

        VulkanDevice& device = world.getDevice();
        MaterialManager& materialManager = world.materialManager;

        materialManager.addTexture("brick_color", std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, "assets/textures/brick/color.jpg")));
        materialManager.addTexture("brick_normal", std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, "assets/textures/brick/normal.png")));

        materialManager.addTexture("wood_color", std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, "assets/textures/wood/color.jpg")));
        materialManager.addTexture("wood_normal", std::unique_ptr<VulkanTexture>(VulkanObject::createTexture(device, "assets/textures/wood/normal.png")));

        {
            Material material{};

            material.albedoIndex = materialManager.getTextureId("brick_color");
            material.normalIndex = materialManager.getTextureId("brick_normal");

            materialManager.addMaterial("brick", material);
        }
        {
            Material material{};

            material.albedoIndex = materialManager.getTextureId("wood_color");
            material.normalIndex = materialManager.getTextureId("wood_normal");

            materialManager.addMaterial("wood", material);
        }

        materialManager.updateGPUBuffer();

        world.parseImages(world.convertImages(materialManager.getTextureItems()));
    }

    bool Game::isToggled(auto key) {
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