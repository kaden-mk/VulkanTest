
#include "VulkanDevice.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace VkRenderer {
    enum DescriptorBindings {
        STORAGE_BINDING,
        SAMPLER_BINDING,
        IMAGE_BINDING
    };

    // TODO: figure out how to query the max values properly with the device

    constexpr int STORAGE_COUNT = 65536;
    constexpr int SAMPLER_COUNT = 65536;
    constexpr int IMAGE_COUNT = 65536;

    class VulkanDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VulkanDevice& device) : device{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            Builder& setBindings(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> newBindings);
            Builder& setFlags(VkDescriptorSetLayoutCreateFlags flags);
            Builder& setPNext(VkDescriptorSetLayoutBindingFlagsCreateInfo& bindingFlags);
            
            std::unique_ptr<VulkanDescriptorSetLayout> build() const;
        private:
            VulkanDevice& device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
            VkDescriptorSetLayoutCreateFlags layoutFlags = 0;
            const void* pNext = nullptr;
        };

        VulkanDescriptorSetLayout(
            VulkanDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayoutCreateFlags flags, const void* pNext);
        ~VulkanDescriptorSetLayout();
        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
        VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        VulkanDevice& device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
        VkDescriptorSetLayoutCreateFlags layoutFlags;

        friend class VulkanDescriptorWriter;
    };

    class VulkanDescriptorPool {
    public:
        class Builder {
        public:
            Builder(VulkanDevice& device) : device{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolSizes(std::vector<VkDescriptorPoolSize> poolSize);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlagBits flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VulkanDescriptorPool> build() const;

        private:
            VulkanDevice& device;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlagBits poolFlags{};
        };

        VulkanDescriptorPool(
            VulkanDevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlagBits poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VulkanDescriptorPool();
        VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
        VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        VulkanDevice& device;
        VkDescriptorPool descriptorPool;

        friend class VulkanDescriptorWriter;
    };

    class VulkanDescriptorWriter {
    public:
        VulkanDescriptorWriter(VulkanDescriptorSetLayout& setLayout, VulkanDescriptorPool& pool);

        VulkanDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VulkanDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VulkanDescriptorSetLayout& setLayout;
        VulkanDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}