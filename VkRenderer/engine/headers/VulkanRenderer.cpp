#include <cassert>

#include "VulkanRenderer.hpp"

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

#include <thread>
#include <chrono>

namespace VkRenderer {
	VulkanRenderer::VulkanRenderer(VulkanWindow &windowToSet, VulkanDevice &deviceToSet) : window(windowToSet), device(deviceToSet)
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	VulkanRenderer::~VulkanRenderer()
	{
		freeCommandBuffers();
	}

	VkCommandBuffer VulkanRenderer::beginFrame()
	{
		assert(!isFrameStarted && "Frame has already started");

		auto result = swapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swapchain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	void VulkanRenderer::endFrame()
	{
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");

		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
			window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swapchain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	bool VulkanRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't begin the render pass when the frame isn't in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkExtent2D extent = swapChain->getSwapChainExtent();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapChain->getRenderPass();
		renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		return true;
	}

	void VulkanRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) const
	{
		assert(isFrameStarted && "Can't end the render pass when the frame isn't in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);
	}

	void VulkanRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<float>(commandBuffers.size()), commandBuffers.data());

		commandBuffers.clear();
	}

	void VulkanRenderer::createCommandBuffers()
	{
		commandBuffers.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = device.getCommandPool();
		allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocateInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate info to commandbuffers!");
		}
	}

	void VulkanRenderer::recreateSwapChain()
	{
		VkExtent2D extent = window.getExtent();

		// REALLY shitty fix for minimizing, TODO: to improve?
		int width;
		int height;

		glfwGetFramebufferSize(window.getWindow(), &width, &height);

		while (width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(window.getWindow(), &width, &height);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		vkDeviceWaitIdle(device.device());

		if (swapChain == nullptr) {
			swapChain = std::make_unique<VulkanSwapChain>(device, extent);
		}
		else {
			std::shared_ptr<VulkanSwapChain> oldSwapChain = std::move(swapChain);
			swapChain = std::make_unique<VulkanSwapChain>(device, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*swapChain.get()))
				throw std::runtime_error("Swapchain image/depth format has changed");
		}
	}
}