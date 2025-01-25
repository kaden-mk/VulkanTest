#pragma once

#include <iostream>
#include <map>
#include <optional>
#include <vector>
#include <array>
#include <stdexcept>
#include <set>
#include <cstdlib>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>

#include "VulkanWindow.hpp"

namespace Engine {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    static class VulkanRenderer
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        static constexpr const char* WINDOW_NAME = "Vulkan";

        VulkanRenderer();
        ~VulkanRenderer();

        VulkanRenderer(const VulkanRenderer&) = delete;
        VulkanRenderer& operator=(const VulkanRenderer&) = delete;

        void run();
    private:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkSwapchainKHR swapChain;
        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        void initVulkan();
        void createCommandBuffer();
        void createCommandPool();
        void createFramebuffers();
        void createRenderPass();
        void createGraphicsPipeline();
        void createInstance();
        void createLogicalDevice();
        void createSwapChain();
        void createSyncObjects();
        void pickPhysicalDevice();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void setupDebugMessenger();
        void createSurface();
        void mainLoop();
        void drawFrame();
        void createImageViews();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        VkShaderModule createShaderModule(const std::vector<char>& code);

        bool isDeviceSuitable(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        bool checkValidationLayerSupport();

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VulkanWindow window{WIDTH, HEIGHT, WINDOW_NAME};

        std::vector<const char*> getRequiredExtensions();

        static std::vector<char> readFile(const std::string& filename);
    };
}