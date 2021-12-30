#pragma once
#include "Types.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <variant>
#include <vector>

class VulkanDestroyer {
    using VulkanObject = std::variant<VkRenderPass, VkFramebuffer, VkImageView, VkImage, VkDeviceMemory, VkPipeline, VkPipelineLayout, VkDescriptorSetLayout, VkDescriptorPool, VkSampler, VkShaderModule, VkCommandPool, VkSwapchainKHR, VkFence, VkSemaphore, AllocatedBuffer, AllocatedImage>;
    struct Destroy {
        Destroy(const VkDevice& dev, const VmaAllocator& alloc) : device(dev), allocator(alloc) {}
        void operator()(VkRenderPass obj) { vkDestroyRenderPass(device, obj, nullptr); }
        void operator()(VkFramebuffer obj) { vkDestroyFramebuffer(device, obj, nullptr); }
        void operator()(VkImageView obj) { vkDestroyImageView(device, obj, nullptr); }
        void operator()(VkImage obj) { vkDestroyImage(device, obj, nullptr); }
        void operator()(VkDeviceMemory obj) { vkFreeMemory(device, obj, nullptr); }
        void operator()(VkPipeline obj) { vkDestroyPipeline(device, obj, nullptr); }
        void operator()(VkPipelineLayout obj) { vkDestroyPipelineLayout(device, obj, nullptr); }
        void operator()(VkDescriptorSetLayout obj) { vkDestroyDescriptorSetLayout(device, obj, nullptr); }
        void operator()(VkDescriptorPool obj) { vkDestroyDescriptorPool(device, obj, nullptr); }
        void operator()(VkSampler obj) { vkDestroySampler(device, obj, nullptr); }
        void operator()(VkShaderModule obj) { vkDestroyShaderModule(device, obj, nullptr); }
        void operator()(VkCommandPool obj) { vkDestroyCommandPool(device, obj, nullptr); }
        void operator()(VkSwapchainKHR obj) { vkDestroySwapchainKHR(device, obj, nullptr); }
        void operator()(VkFence obj) { vkDestroyFence(device, obj, nullptr); }
        void operator()(VkSemaphore obj) { vkDestroySemaphore(device, obj, nullptr); }
        void operator()(AllocatedBuffer obj) { vmaDestroyBuffer(allocator, obj.buffer, obj.allocation); }
        void operator()(AllocatedImage obj) { vmaDestroyImage(allocator, obj.image, obj.allocation); }
    private:
        const VkDevice& device;
        const VmaAllocator& allocator;
    };
public:
    VulkanDestroyer(const VkDevice& device, const VmaAllocator& allocator) : destroy(device, allocator) {}
    void Add(VulkanObject obj) { objects.push_back(obj); }
    template <typename T>
    void Add(std::vector<T> objects) { 
        for (auto& obj : objects) { 
            Add(obj);
        } 
    }
    void DestroyAll() {
        for (auto& obj : objects) {
            std::visit(destroy, obj);
        }
    }

private:
    std::vector<VulkanObject> objects;
    Destroy destroy;
};