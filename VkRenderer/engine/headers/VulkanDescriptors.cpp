#include "VulkanDescriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace VkRenderer {

    // *************** Descriptor Set Layout Builder *********************

    VulkanDescriptorSetLayout::Builder& VulkanDescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");

        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;

        return *this;
    }

    VulkanDescriptorSetLayout::Builder& VulkanDescriptorSetLayout::Builder::setBindings(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> newBindings)
    {
        bindings = newBindings;

        return *this;
    }

    VulkanDescriptorSetLayout::Builder& VulkanDescriptorSetLayout::Builder::setFlags(VkDescriptorSetLayoutCreateFlags flags)
    {
        layoutFlags = flags;
        return *this;
    }

    VulkanDescriptorSetLayout::Builder& VulkanDescriptorSetLayout::Builder::setPNext(VkDescriptorSetLayoutBindingFlagsCreateInfo& bindingFlags)
    {
        pNext = &bindingFlags;
        return *this;
    }

    std::unique_ptr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayout::Builder::build() const {
        return std::make_unique<VulkanDescriptorSetLayout>(device, bindings, layoutFlags, pNext);
    }

    // *************** Descriptor Set Layout *********************

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayoutCreateFlags flags, const void* pNext) : device{ device }, bindings{ bindings }, layoutFlags{ flags } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto& kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.pNext = pNext;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
        descriptorSetLayoutInfo.flags = layoutFlags;

        if (vkCreateDescriptorSetLayout(
            device.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
        //poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::setPoolSizes(std::vector<VkDescriptorPoolSize> poolSize) {
        poolSizes = poolSize;
        return *this;
    }

    VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlagBits flags) {
        poolFlags = flags;
        return *this;
    }

    VulkanDescriptorPool::Builder& VulkanDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPool::Builder::build() const {
        return std::make_unique<VulkanDescriptorPool>(device, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice& device, uint32_t maxSets, VkDescriptorPoolCreateFlagBits poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes) : device{ device } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = poolSizes.size();
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    VulkanDescriptorPool::~VulkanDescriptorPool() {
        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
    }

    bool VulkanDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void VulkanDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            device.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void VulkanDescriptorPool::resetPool() {
        vkResetDescriptorPool(device.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    VulkanDescriptorWriter::VulkanDescriptorWriter(VulkanDescriptorSetLayout& setLayout, VulkanDescriptorPool& pool) : setLayout{ setLayout }, pool{ pool } {}

    VulkanDescriptorWriter& VulkanDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];
        
        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    VulkanDescriptorWriter& VulkanDescriptorWriter::writeBuffers(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings, VkDescriptorBufferInfo* bufferInfo)
    {
        for (auto& kv : bindings) {
            VkDescriptorSetLayoutBinding binding = kv.second;

            assert(setLayout.bindings.count(binding.binding) == 1 && "Layout does not contain specified binding");

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = binding.descriptorType;
            write.dstBinding = binding.binding;
            write.pBufferInfo = bufferInfo;
            write.descriptorCount = binding.descriptorCount;

            writes.push_back(write);
        }

        return *this;
    }

    VulkanDescriptorWriter& VulkanDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    VulkanDescriptorWriter& VulkanDescriptorWriter::writeImageArray(uint32_t binding, VkDescriptorImageInfo* images, uint32_t count)
    {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        
        auto& bindingDescription = setLayout.bindings[binding];
        assert(bindingDescription.descriptorCount >= count && "Too many images for binding");
        assert(bindingDescription.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && "Wrong descriptor type");
        
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = count;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = images;

        writes.push_back(write);
        return *this;
    }

    bool VulkanDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void VulkanDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.device.device(), writes.size(), writes.data(), 0, nullptr);
    }

}