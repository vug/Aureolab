#pragma once

#include "Instance.h"

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

		const Instance& instance;
		const VulkanWindow& win;

		operator VkSurfaceKHR () const { return handle; }
		operator VkSurfaceKHR* () { return &handle; }
		operator VkSurfaceKHR& () { return handle; }
	};
}