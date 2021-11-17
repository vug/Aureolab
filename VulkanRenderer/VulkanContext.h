#pragma once

#include "VulkanWindow.h"

#include <vulkan/vulkan.h>

class VulkanContext {
public:
	VulkanContext(VulkanWindow& win, bool validation = true);
	~VulkanContext();

	static VkInstance& CreateInstance(uint32_t requestedExtensionCount, const char** requestedExtensions, bool enableValidationLayers, VkDebugUtilsMessengerEXT& outDebugMessenger);
	static VkSurfaceKHR& CreateSurface(VulkanWindow& win, VkInstance& instance);
private:
	// Vulkan Objects that needs to be destroyed with VulkanContext
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkInstance instance;
	bool shouldDestroyDebugUtils;

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};