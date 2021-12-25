#pragma once

#include "VulkanWindow.h"
#include "VulkanDestroyer.h"
#include "Types.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <functional> // reference_wrapper
#include <optional>
#include <queue>
#include <vector>
#include <tuple>
#include <memory>

namespace vr {
	struct Instance {
		InstanceBuilder builder = {};
		VkInstance handle = VK_NULL_HANDLE;
		operator VkInstance() const;
	};

	// The idea behind a builder is that it's a class that manages all resources required to create an object
	// in its local scope. 
	// VkInstanceCreateInfo::pApplicationInfo is a pointer to VkApplicationInfo
	// If we have a function that creates and returns a VkInstanceCreateInfo, which uses a VkApplicationInfo created locally
	// that'll be destroyed at function return.
	class InstanceBuilder {
	public:
		InstanceBuilder();
		Instance build();
		VkApplicationInfo appInfo = {};
	};
}

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

	bool isAdequate() {
		return !formats.empty() && !presentModes.empty();
	}
};

struct SwapchainInfo {
	VkSurfaceFormatKHR surfaceFormat;
	VkExtent2D extent;
	std::vector<VkImageView> imageViews;
	VkFormat depthFormat;
};

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
	VulkanContext(VulkanWindow& win, bool validation = true);
	~VulkanContext();

	// These functions return individual Vulkan objects for the initialization part that'll be the same for all Vulkan Apps
	// They are all static to make the creation dependencies explicit, i.e. cannot access internal state hence inputs and outputs have to be declarated
	// They return references to Vulkan Objects that are known to be not deleted at the end of function scope (intermediate structs of VulkanContext such as SwapChainSupportDetails are copied over)
	// Some returns a tuple of various types, first being the Vulkan Object mentioned in function name the rest are side objects that comes with it.
	//   Later they might return wrappers with getters. Ex: PDevice->GetQueueIndices(), Device->GetGraphicsQueue() etc
	
	// (Used in VulkanContext construction)
	static std::tuple<VkInstance, VkDebugUtilsMessengerEXT, std::vector<const char*>> CreateInstance(uint32_t requestedExtensionCount, const char** requestedExtensions, bool enableValidationLayers);
	static VkSurfaceKHR CreateSurface(VulkanWindow& win, VkInstance& instance);
	// Search and pick a suitable GPU with needed properties. Also returns the queue families on that device
	static std::tuple<VkPhysicalDevice, QueueFamilyIndices, SwapChainSupportDetails, std::vector<const char*>> CreatePhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface);
	static std::tuple<VkDevice, VkQueue, VkQueue> CreateLogicalDevice(VkPhysicalDevice& physicalDevice, QueueFamilyIndices& queueIndices, std::vector<const char*>& requiredExtensions, bool enableValidationLayers, std::vector<const char*>& vulkanLayers);
	static std::tuple<VmaAllocator, std::unique_ptr<VulkanDestroyer>> CreateAllocatorAndDestroyer(const VkInstance& instance, const VkPhysicalDevice& physicalDevice, const VkDevice& device);
	static std::tuple<VkSwapchainKHR, SwapchainInfo> CreateSwapChain(VkDevice& device, VkSurfaceKHR& surface, QueueFamilyIndices& queueIndices, SwapChainSupportDetails& swapChainSupportDetails);

	// Can be used by Example apps
	static VkCommandPool CreateGraphicsCommandPool(const VkDevice& device, uint32_t graphicsQueueFamilyIndex);
	static FrameSyncCmd CreateFrameSyncCmd(const VkDevice& device, uint32_t graphicsQueueFamilyIndex);
	static VkCommandBuffer CreateCommandBuffer(const VkDevice& device, const VkCommandPool& cmdPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	static VkFramebuffer CreateFramebuffer(const VkDevice& device, const VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, const VkExtent2D& extent);
	static std::tuple<std::vector<VkFramebuffer>, VkImageView, AllocatedImage&> CreateSwapChainFrameBuffers(const VkDevice& device, const VmaAllocator& allocator, const VkRenderPass& renderPass, const SwapchainInfo&);
	static AllocatedBuffer CreateAllocatedBuffer(const VmaAllocator& allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage); 
	static VkDescriptorPool CreateDescriptorPool(const VkDevice& device, const std::vector<VkDescriptorPoolSize>& sizes);

	const VkDevice& GetDevice() const { return device; }
	const VkQueue& GetGraphicsQueue() const { return graphicsQueue; }
	const VkQueue& GetPresentationQueue() const { return presentQueue; }
	const QueueFamilyIndices& GetQueueFamilyIndices() const { return queueIndices; }
	const VkSwapchainKHR& GetSwapchain() const { return swapchain; }
	const SwapchainInfo& GetSwapchainInfo() const { return swapchainInfo; }
	const VmaAllocator& GetAllocator() const { return vmaAllocator; }
	VulkanDestroyer& GetDestroyer() const { return *destroyer; }

	// 1) acquire (next available) image from swap chain
	// 2) clears command buffer, records into it via cmdFunc with "one-time" option
	// 2.5) executes it in given RenderPass with that image as attachment in the framebuffer
	// 3) return the image to the swapchain for presentation
	// If there is only one FrameSyncCmd it'll be blocked, i.e. acquisition, queue processing and presentation happens sequentially. CPU will wait for GPU to finish before creating commands for the next frame.
	static void drawFrame(const VkDevice& device, const VkSwapchainKHR& swapchain, const VkQueue& graphicsQueue, const VkRenderPass& renderPass, const std::vector<std::shared_ptr<IFrameData>>& frames, const std::vector<VkFramebuffer>& swapchainFramebuffers, const SwapchainInfo& swapchainInfo, const std::vector<VkClearValue>& clearValues, std::function<void(const VkCommandBuffer&, uint32_t frameNo)> cmdFunc);

	virtual void OnResize(int width, int height) override;
private:
	// Vulkan Objects that needs to be destroyed with VulkanContext
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	VkInstance instance = VK_NULL_HANDLE;
	bool shouldDestroyDebugUtils = false;
	//
	VmaAllocator vmaAllocator; //vma lib allocator
	std::unique_ptr<VulkanDestroyer> destroyer;

	SwapchainInfo swapchainInfo;
	// Queues into which commands will be submitted by client app
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	QueueFamilyIndices queueIndices;

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};
