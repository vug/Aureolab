#pragma once

#include "Instance.h"
#include "Surface.h"

#include <vulkan/vulkan.h>

#include <functional>
#include <optional>
#include <vector>

namespace vr {
	class QueueFamilyIndices {
	public:
		QueueFamilyIndices(VkQueueFlagBits requestedQueues = VK_QUEUE_GRAPHICS_BIT);

		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> transferFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> presentFamily;

		VkQueueFlagBits requestedQueues = {};

		bool isComplete();
		void logIndices();
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		bool isAdequate() {
			return !formats.empty() && !presentModes.empty();
		}
	};

	struct SwapchainInfo {
		VkSurfaceFormatKHR surfaceFormat = {};
		VkExtent2D extent = {};
		std::vector<VkImageView> imageViews;
		VkFormat depthFormat = {};
	};

	// Search and pick a suitable GPU with needed properties. Also returns the queue families on that device
	class PhysicalDeviceBuilder {
	public:
		// TODO: more parameters such as requried extensions
		PhysicalDeviceBuilder(const Instance& instance, const Surface& surface, std::function<bool(VkPhysicalDeviceFeatures)> devFeaturesFunc = defaultDevFeaturesFunc);

		const Instance& instance;
		const Surface& surface;

		QueueFamilyIndices indices = {};
		std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		SwapChainSupportDetails swapchainSupportDetails = {};
		std::vector<VkPhysicalDevice> physicalDevices;
		std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties;
		std::vector<VkPhysicalDeviceFeatures> physicalDeviceFeatures;

		static bool defaultDevFeaturesFunc(VkPhysicalDeviceFeatures devFeats) {
			return devFeats.samplerAnisotropy && devFeats.fillModeNonSolid;
		};
	};

	struct PhysicalDevice {
		PhysicalDevice(const PhysicalDeviceBuilder& builder);
		// VkPhysicalDevice does not need to be destroyed
		PhysicalDeviceBuilder builder;
		VkPhysicalDevice handle = VK_NULL_HANDLE;

		const Instance& instance;
		const Surface& surface;

		operator VkPhysicalDevice () const { return handle; }
		operator VkPhysicalDevice* () { return &handle; }
		operator VkPhysicalDevice& () { return handle; }
	};
}