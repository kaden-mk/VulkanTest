#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <map>
#include <optional>
#include <vector>
#include <array>
#include <stdexcept>
#include <cstdlib>

namespace Engine {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;

        bool isComplete() {
            return graphicsFamily.has_value();
        }
    };

    class VulkanRenderer
    {
        public:
            VulkanRenderer();
            ~VulkanRenderer();

	    private:
            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice device;
            VkQueue graphicsQueue;

            void initVulkan();
            void createInstance();
            void createLogicalDevice();
            void pickPhysicalDevice();
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
            void setupDebugMessenger();
            
            bool isDeviceSuitable(VkPhysicalDevice device);
            bool checkValidationLayerSupport();

            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

            std::vector<const char*> getRequiredExtensions();

    };
}