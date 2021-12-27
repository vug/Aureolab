#pragma once

#include "VulkanInstance.h"

#include "../VulkanWindow.h"

#include <vulkan/vulkan.h>

namespace vr {
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