#include "Device.h"

#include "Core/Log.h"

#include <set>

namespace vr {
	DeviceBuilder::DeviceBuilder(const PhysicalDevice& physicalDevice)
		: physicalDevice(physicalDevice) {
        const QueueFamilyIndices& queueIndices = physicalDevice.builder.indices;

        {
            std::set<uint32_t> uniqueQueueFamilies =
                { queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() };

            float queuePriority = 1.0f; // determine scheduling of command buffer execution
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }
        }

        // TODO: Make "device features to be used / enabled" parameters
        deviceFeatures.samplerAnisotropy = VK_TRUE; // use anisotropy filters for textures

        const std::vector<const char*>& requiredExtensions = physicalDevice.builder.requiredExtensions;
        const std::vector<const char*>& layers = physicalDevice.builder.instance.builder.layers;

        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        info.pQueueCreateInfos = queueCreateInfos.data();
        info.pEnabledFeatures = &deviceFeatures;
        info.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        info.ppEnabledExtensionNames = requiredExtensions.data();
        if (physicalDevice.builder.instance.builder.params.validation) {
            info.enabledLayerCount = static_cast<uint32_t>(layers.size());
            info.ppEnabledLayerNames = layers.data();
        }
        else {
            info.enabledLayerCount = 0;
        }
	}

    // ---------------------

	Device::Device(const DeviceBuilder& builder) 
		: builder(builder), physicalDevice(builder.physicalDevice) {
        Log::Debug("Creating Logical Device...");

        VkResult result = vkCreateDevice(builder.physicalDevice, &builder.info, nullptr, &handle);
        if (result != VK_SUCCESS) {
            Log::Debug("Failed to create logical device!");
        }

        const QueueFamilyIndices& queueIndices = builder.physicalDevice.builder.indices;
        vkGetDeviceQueue(handle, queueIndices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(handle, queueIndices.presentFamily.value(), 0, &presentQueue);
	}

	Device::~Device() {
        Log::Debug("Destroying Logical Device...");
        vkDestroyDevice(handle, nullptr);
	}
}