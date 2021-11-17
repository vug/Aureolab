#pragma once

#include "VulkanWindow.h"

#include <vulkan/vulkan.h>

class VulkanContext {
public:
	VulkanContext(VulkanWindow& win, bool validation = true);
	~VulkanContext();

	static VkInstance& CreateInstance(uint32_t requestedExtensionCount, const char** requestedExtensions, bool enableValidationLayers, VkDebugUtilsMessengerEXT& outDebugMessenger);
private:
	// Vulkan Objects that needs to be destroyed with VulkanContext
	VkDebugUtilsMessengerEXT debugMessenger;
	VkInstance instance;
	bool shouldDestroyDebugUtils;

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};