#pragma once

#include "VulkanWindow.h"
#include "VulkanObjects.h"
#include "VulkanDestroyer.h"
#include "Types.h"

#include <vulkan/vulkan.h>

#include <queue>
#include <vector>
#include <tuple>
#include <memory>

// Keeps objects together that are required for queueing up draw commands in a synchronized way per frame
struct FrameSyncCmd {
	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;
	
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
};

class IFrameData {
public:
	IFrameData(const FrameSyncCmd& syncCmd1) : syncCmd(syncCmd1) {}

	const FrameSyncCmd& GetFrameSyncCmdData() const {
		return syncCmd;
	}
private:
	FrameSyncCmd syncCmd;
};

class ImmediateCommandSubmitter {
public:
	ImmediateCommandSubmitter(const VkDevice& device, const VkQueue& graphicsQueue, const uint32_t graphicsQueueFamilyIndex, VulkanDestroyer& destroyer);
	void Submit(std::function<void(VkCommandBuffer cmd)>&& function);
private:
	VkFence uploadFence;
	VkCommandPool cmdPool;
	VkDevice device;
	VkQueue queue;
};


class VulkanContext : public IResizable {
public:
	VulkanContext(VulkanWindow& win);
	~VulkanContext();

	// Order of members is important, which is the order in which Vulkan Objects are generated. 
	// When VulkanContext is destructed, their destructors will be called in reverse order.
	// Objects whose classes don't have default constructors will be initialized in the initialization list.
	vr::Instance instance;
	std::unique_ptr<vr::DebugMessenger> debugMessenger = nullptr;
	vr::Surface surface;
	vr::PhysicalDevice physicalDevice;
	vr::Device device;
	vr::Allocator allocator;
	std::unique_ptr<VulkanDestroyer> destroyer;
	vr::Swapchain swapchain;

	// These functions return individual Vulkan objects for the initialization part that'll be the same for all Vulkan Apps
	// They are all static to make the creation dependencies explicit, i.e. cannot access internal state hence inputs and outputs have to be declarated
	// They return references to Vulkan Objects that are known to be not deleted at the end of function scope (intermediate structs of VulkanContext such as SwapChainSupportDetails are copied over)
	// Some returns a tuple of various types, first being the Vulkan Object mentioned in function name the rest are side objects that comes with it.
	//   Later they might return wrappers with getters. Ex: PDevice->GetQueueIndices(), Device->GetGraphicsQueue() etc
	
	// Can be used by Example apps
	static VkCommandPool CreateGraphicsCommandPool(const VkDevice& device, uint32_t graphicsQueueFamilyIndex);
	static FrameSyncCmd CreateFrameSyncCmd(const VkDevice& device, uint32_t graphicsQueueFamilyIndex);
	static VkCommandBuffer CreateCommandBuffer(const VkDevice& device, const VkCommandPool& cmdPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	static VkFramebuffer CreateFramebuffer(const VkDevice& device, const VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, const VkExtent2D& extent);
	static std::tuple<std::vector<VkFramebuffer>, VkImageView, AllocatedImage&> CreateSwapChainFrameBuffers(const VkDevice& device, const VmaAllocator& allocator, const VkRenderPass& renderPass, const vr::SwapchainInfo&);
	static AllocatedBuffer CreateAllocatedBuffer(const VmaAllocator& allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage); 
	static VkDescriptorPool CreateDescriptorPool(const VkDevice& device, const std::vector<VkDescriptorPoolSize>& sizes);

	const VkDevice& GetDevice() const { return device; }
	const VkQueue& GetGraphicsQueue() const { return device.graphicsQueue; }
	const VkQueue& GetPresentationQueue() const { return device.presentQueue; }
	const vr::QueueFamilyIndices& GetQueueFamilyIndices() const { return physicalDevice.builder.indices; }
	const VkSwapchainKHR& GetSwapchain() const { return swapchain; }
	const vr::SwapchainInfo& GetSwapchainInfo() const { return swapchain.swapchainInfo; }
	const VmaAllocator& GetAllocator() const { return allocator; }
	VulkanDestroyer& GetDestroyer() const { return *destroyer; }

	// 1) acquire (next available) image from swap chain
	// 2) clears command buffer, records into it via cmdFunc with "one-time" option
	// 2.5) executes it in given RenderPass with that image as attachment in the framebuffer
	// 3) return the image to the swapchain for presentation
	// If there is only one FrameSyncCmd it'll be blocked, i.e. acquisition, queue processing and presentation happens sequentially. CPU will wait for GPU to finish before creating commands for the next frame.
	static void drawFrame(const VkDevice& device, const VkSwapchainKHR& swapchain, const VkQueue& graphicsQueue, const VkRenderPass& renderPass, const std::vector<std::shared_ptr<IFrameData>>& frames, const std::vector<VkFramebuffer>& swapchainFramebuffers, const vr::SwapchainInfo& swapchainInfo, const std::vector<VkClearValue>& clearValues, std::function<void(const VkCommandBuffer&, uint32_t frameNo)> cmdFunc);

	virtual void OnResize(int width, int height) override;
};
