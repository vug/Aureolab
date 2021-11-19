#pragma once
#include <vulkan/vulkan.h>

#include <variant>
#include <vector>

class VulkanDestroyer {
    using VulkanObject = std::variant<VkRenderPass, VkFramebuffer, VkPipeline, VkPipelineLayout, VkShaderModule>;
    struct Destroy {
        Destroy(const VkDevice& dev) : device(dev) {}
        void operator()(VkRenderPass obj) { vkDestroyRenderPass(device, obj, nullptr); }
        void operator()(VkFramebuffer obj) { vkDestroyFramebuffer(device, obj, nullptr); }
        void operator()(VkPipeline obj) { vkDestroyPipeline(device, obj, nullptr); }
        void operator()(VkPipelineLayout obj) { vkDestroyPipelineLayout(device, obj, nullptr); }
        void operator()(VkShaderModule obj) { vkDestroyShaderModule(device, obj, nullptr); }
    private:
        const VkDevice& device;
    };
public:
    VulkanDestroyer(const VkDevice& device) : destroy(device) {}
    void Add(VulkanObject obj) { objects.push_back(obj); }
    void DestroyAll() {
        for (auto& obj : objects) {
            std::visit(destroy, obj);
        }
    }

private:
    std::vector<VulkanObject> objects;
    Destroy destroy;
};