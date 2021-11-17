#pragma once

#include "VulkanWindow.h"

#include <vulkan/vulkan.h>

#include <optional>
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

	static std::tuple<VkInstance&, VkDebugUtilsMessengerEXT&> CreateInstance(uint32_t requestedExtensionCount, const char** requestedExtensions, bool enableValidationLayers);
	static VkSurfaceKHR& CreateSurface(VulkanWindow& win, VkInstance& instance);
	static VkPhysicalDevice& CreatePhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface);
	// Search and pick a suitable GPU with needed properties. Also returns the queue families on that device
	static std::tuple<VkPhysicalDevice&, QueueFamilyIndices> CreatePhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface);
	static VkDevice& CreateLogicalDevice();
private:
	// Vulkan Objects that needs to be destroyed with VulkanContext
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	VkInstance instance = VK_NULL_HANDLE;
	bool shouldDestroyDebugUtils = false;

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};