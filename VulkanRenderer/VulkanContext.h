#pragma once

#include "VulkanWindow.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>
#include <tuple>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class VulkanContext {
public:
	VulkanContext(VulkanWindow& win, bool validation = true);
	~VulkanContext();

	// These functions return individual Vulkan objects for the initialization part that'll be the same for all Vulkan Apps
	// They are all static to make the creation dependencies explicit, i.e. cannot access internal state hence inputs and outputs have to be declarated
	// They return references to Vulkan Objects that are known to be not deleted at the end of function scope
	// Some returns a tuple of various types, first being the Vulkan Object mentioned in function name the rest are side objects that comes with it.
	//   Later they might return wrappers with getters. Ex: PDevice->GetQueueIndices(), Device->GetGraphicsQueue() etc

	static std::tuple<VkInstance&, VkDebugUtilsMessengerEXT&, std::vector<const char*>&> CreateInstance(uint32_t requestedExtensionCount, const char** requestedExtensions, bool enableValidationLayers);
	static VkSurfaceKHR& CreateSurface(VulkanWindow& win, VkInstance& instance);
	// Search and pick a suitable GPU with needed properties. Also returns the queue families on that device
	static std::tuple<VkPhysicalDevice&, QueueFamilyIndices, std::vector<const char*>&> CreatePhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface);
	static std::tuple<VkDevice&, VkQueue&, VkQueue&> CreateLogicalDevice(VkPhysicalDevice& physicalDevice, QueueFamilyIndices& queueIndices, std::vector<const char*>& requiredExtensions, bool enableValidationLayers, std::vector<const char*>& vulkanLayers);

	const VkQueue& GetGraphicsQueue() const { return graphicsQueue; }
	const VkQueue& GetPresentationQueue() const { return presentQueue; }
private:
	// Vulkan Objects that needs to be destroyed with VulkanContext
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	VkInstance instance = VK_NULL_HANDLE;
	bool shouldDestroyDebugUtils = false;

	// Queues into which commands will be submitted by client app
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};
