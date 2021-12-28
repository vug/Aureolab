#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#include <vector>

namespace vr {
	class SwapchainBuilder {
	public:
		SwapchainBuilder(const Device& device);
		const Device& device;

		VkSurfaceFormatKHR surfaceFormat = {};
		VkPresentModeKHR presentMode;
		VkExtent2D swapExtent = {};
		uint32_t imageCount = 0;
		VkImageViewCreateInfo imageViewInfo = {};
		VkSwapchainCreateInfoKHR info = {};
	};

	struct Swapchain {
		Swapchain(const SwapchainBuilder& builder);
		~Swapchain();
		SwapchainBuilder builder;

		SwapchainInfo swapchainInfo = {};
		std::vector<VkImage> imageHandles;

		VkSwapchainKHR handle = VK_NULL_HANDLE;
		operator VkSwapchainKHR () const { return handle; }
		operator VkSwapchainKHR* () { return &handle; }
		operator VkSwapchainKHR& () { return handle; }
	};
}