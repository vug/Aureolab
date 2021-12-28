#pragma once

#include "PhysicalDevice.h"

namespace vr {
	class DeviceBuilder {
	public:
		DeviceBuilder(const PhysicalDevice& physicalDevice);
		const PhysicalDevice& physicalDevice;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo info = {};
	};

	struct Device {
		Device(const DeviceBuilder& builder);
		~Device();
		DeviceBuilder builder;
		VkDevice handle = VK_NULL_HANDLE;

		const PhysicalDevice& physicalDevice;

		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;

		operator VkDevice& () { return handle; }
		operator const VkDevice& () const { return handle; }
	};
}
