#pragma once

#include "VulkanWindow.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace vr {
	class InstanceBuilder {
	public:
		// Defaults for a windowed Windows app with validation layers
		struct Params {
			bool headless = false;
			bool validation = true;
			std::vector<const char*> requestedLayers = {};
			std::vector<const char*> requestedExtensions = {};
		};

		InstanceBuilder(const Params& params = Params());
		operator const VkInstanceCreateInfo* ();

	private:
		std::vector<const char*> initLayers(const Params& params);
		std::vector<const char*> initExtensions(const Params& params);
		VkApplicationInfo initAppInfo(const Params& params);
		VkInstanceCreateInfo initInfo(const Params& params);

	public:
		Params params = {};
		std::vector<const char*> layers;
		std::vector<const char*> extensions;
		VkApplicationInfo appInfo = {};
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		VkInstanceCreateInfo info = {};
	};

	struct Instance {
		Instance(const InstanceBuilder& builder);
		~Instance();
		InstanceBuilder builder = {};
		VkInstance handle = VK_NULL_HANDLE;

		operator VkInstance () const { return handle; }
		operator VkInstance* () { return &handle; }
		operator VkInstance& () { return handle; }
	};

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
	};

	class SurfaceBuilder {
	public:
		SurfaceBuilder(const Instance& instance, const VulkanWindow& win);

		const Instance& instance;
		const VulkanWindow& win;
	};

	struct Surface {
		Surface(const SurfaceBuilder& builder);
		~Surface();
		SurfaceBuilder builder;
		VkSurfaceKHR handle = VK_NULL_HANDLE;

		operator VkSurfaceKHR () const { return handle; }
		operator VkSurfaceKHR* () { return &handle; }
		operator VkSurfaceKHR& () { return handle; }
	};
}