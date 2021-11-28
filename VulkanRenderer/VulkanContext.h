#pragma once

#include "VulkanWindow.h"
#include "Types.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <functional> // reference_wrapper
#include <optional>
#include <queue>
#include <vector>
#include <tuple>

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

class VulkanContext : public IResizable {
public:
	VulkanContext(VulkanWindow& win, bool validation = true);
	~VulkanContext();

	// These functions return individual Vulkan objects for the initialization part that'll be the same for all Vulkan Apps
	// They are all static to make the creation dependencies explicit, i.e. cannot access internal state hence inputs and outputs have to be declarated
	// They return references to Vulkan Objects that are known to be not deleted at the end of function scope (intermediate structs of VulkanContext such as SwapChainSupportDetails are copied over)
	// Some returns a tuple of various types, first being the Vulkan Object mentioned in function name the rest are side objects that comes with it.
	//   Later they might return wrappers with getters. Ex: PDevice->GetQueueIndices(), Device->GetGraphicsQueue() etc

	static std::tuple<VkInstance&, VkDebugUtilsMessengerEXT&, std::vector<const char*>&> CreateInstance(uint32_t requestedExtensionCount, const char** requestedExtensions, bool enableValidationLayers);
	static VkSurfaceKHR& CreateSurface(VulkanWindow& win, VkInstance& instance);
	// Search and pick a suitable GPU with needed properties. Also returns the queue families on that device
	static std::tuple<VkPhysicalDevice&, QueueFamilyIndices, SwapChainSupportDetails, std::vector<const char*>> CreatePhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface);
	static std::tuple<VkDevice&, VkQueue, VkQueue> CreateLogicalDevice(VkPhysicalDevice& physicalDevice, QueueFamilyIndices& queueIndices, std::vector<const char*>& requiredExtensions, bool enableValidationLayers, std::vector<const char*>& vulkanLayers);
	static std::tuple<VkSwapchainKHR&, SwapchainInfo> CreateSwapChain(VkDevice& device, VkSurfaceKHR& surface, QueueFamilyIndices& queueIndices, SwapChainSupportDetails& swapChainSupportDetails);
	static VkCommandPool& CreateGraphicsCommandPool(VkDevice& device, uint32_t graphicsQueueFamilyIndex);
	static VkFramebuffer& CreateFramebuffer(const VkDevice& device, const VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, const VkExtent2D& extent);
	static std::tuple<std::vector<VkFramebuffer>, VkImageView, AllocatedImage&> CreateSwapChainFrameBuffers(const VkDevice& device, const VmaAllocator& allocator, const VkRenderPass& renderPass, const SwapchainInfo&);

	const VkDevice& GetDevice() const { return device; }
	const VkCommandPool& GetCommandPool() const{ return commandPool; }
	const VkQueue& GetGraphicsQueue() const { return graphicsQueue; }
	const VkQueue& GetPresentationQueue() const { return presentQueue; }
	const SwapchainInfo& GetSwapchainInfo() const { return swapchainInfo; }
	const VmaAllocator& GetAllocator() const { return vmaAllocator; }

	// 1) acquire (next available) image from swap chain
	// 2) execute command buffer in given RenderPass with that image as attachment in the framebuffer
	// 3) return the image to the swapchain for presentation
	// Blocked means acquisition, queue processing and presentation happens sequentially
	void drawFrameBlocked(VkRenderPass& renderPass, VkCommandBuffer& cmdBuf, const std::vector<VkFramebuffer>& swapchainFramebuffers, const SwapchainInfo& swapchainInfo, const std::vector<VkClearValue>& clearValues, std::function<void(VkCommandBuffer&)> cmdFunc);

	virtual void OnResize(int width, int height) override;
private:
	// Vulkan Objects that needs to be destroyed with VulkanContext
	VkCommandPool commandPool;
	// VkSwapchainImageView's are stored in swapchainInfo
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	VkInstance instance = VK_NULL_HANDLE;
	bool shouldDestroyDebugUtils = false;
	//
	VmaAllocator vmaAllocator; //vma lib allocator

	SwapchainInfo swapchainInfo;
	// Queues into which commands will be submitted by client app
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	//
	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};
