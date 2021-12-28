#pragma once

#include <vulkan/vulkan.h>

#include "Instance.h"

namespace vr {
	class DebugMessengerBuilder {
	public:
		DebugMessengerBuilder(const Instance& instance);

		const Instance& instance;
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	private:
		PFN_vkCreateDebugUtilsMessengerEXT initCreateFunc();
		PFN_vkDestroyDebugUtilsMessengerEXT initDestroyFunc();
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	};

	struct DebugMessenger {
		DebugMessenger(const DebugMessengerBuilder& builder);
		~DebugMessenger();
		DebugMessengerBuilder builder;
		VkDebugUtilsMessengerEXT handle = VK_NULL_HANDLE;

		const Instance& instance;
	};
}