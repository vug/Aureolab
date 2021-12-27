#include "PhysicalDevice.h"

#include "Core/Log.h"

#include <set>
#include <string>
#include <vector>

namespace vr {
    QueueFamilyIndices::QueueFamilyIndices(VkQueueFlagBits requestedQueues) : requestedQueues(requestedQueues) {}

    bool QueueFamilyIndices::isComplete() {
        bool hasValues = true;
        if (requestedQueues & VK_QUEUE_GRAPHICS_BIT) {
            hasValues &= graphicsFamily.has_value();
        }
        if (requestedQueues & VK_QUEUE_TRANSFER_BIT) {
            hasValues &= transferFamily.has_value();
        }
        if (requestedQueues & VK_QUEUE_COMPUTE_BIT) {
            hasValues &= computeFamily.has_value();
        }
        hasValues &= presentFamily.has_value();
        return hasValues;
    }

    void QueueFamilyIndices::logIndices() {
        if (!isComplete()) {
            Log::Debug("\t\tDoes not have indices for all requested queues");
        }

        if (requestedQueues & VK_QUEUE_GRAPHICS_BIT)
            Log::Debug("\t\tGraphics index: {}", graphicsFamily.value());

        if (requestedQueues & VK_QUEUE_TRANSFER_BIT)
            Log::Debug("\t\tTransfer index: {}", transferFamily.value());

        if (requestedQueues & VK_QUEUE_COMPUTE_BIT)
            Log::Debug("\t\tCompute index: {}", computeFamily.value());

        Log::Debug("\t\tPresent / Surface index : {}.", presentFamily.value());
    }

    // ---------------------------------

	PhysicalDeviceBuilder::PhysicalDeviceBuilder(const Instance& instance, const Surface& surface, std::function<bool(VkPhysicalDeviceFeatures)> devFeaturesFunc)
		: instance(instance), 
		surface(surface) {
		std::vector<VkPhysicalDevice> devices;

        // Available Devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            Log::Critical("Failed to find a GPU with Vulkan support!");
            exit(EXIT_FAILURE);
        }
        devices.resize(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        Log::Debug("Available devices:");
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            Log::Debug("\t{}", properties.deviceName);
        }

        // Gather all suitable devices among available devices
        Log::Debug("Suitable devices:");
        indices = QueueFamilyIndices(VK_QUEUE_GRAPHICS_BIT);

        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            Log::Debug("\tChecking suitability of {}", deviceProperties.deviceName);
            // Check required queues, store queue indices if there are any
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int ix = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = ix;
                }
                if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                    indices.transferFamily = ix;
                }
                if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                    indices.computeFamily = ix;
                }
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, ix, surface, &presentSupport);
                if (presentSupport) { indices.presentFamily = ix; }
                if (indices.isComplete()) { break; }
                ix++;
            }
            if (ix == queueFamilies.size()) {
                Log::Debug("\t\tDoes not have requested queues.");
                continue;
            }
            // invariant: indices.isComplete() == true aka indices is a QueueFamilyIndices with required queues
            Log::Debug("\t\tHas {} queue families.", queueFamilyCount);
            indices.logIndices();

            // Check required extensions
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
            std::set<std::string> extensionsNotAvailable(requiredExtensions.begin(), requiredExtensions.end());
            for (const auto& extension : availableExtensions) { extensionsNotAvailable.erase(extension.extensionName); }
            // not all required extensions are available in this device
            if (!extensionsNotAvailable.empty()) {
                std::string s;
                for (const auto& ext : extensionsNotAvailable) { s += ext + " "; }
                Log::Debug("\t\tMissing required extensions: {}", s);
                continue;
            }
            else {
                std::string s;
                for (const auto& ext : requiredExtensions) { s += std::string(ext) + " "; }
                Log::Debug("\t\tHas extensions required by swapchain: {}", s);
            }

            // Check Swapchain support
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainSupportDetails.capabilities);
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
            if (formatCount != 0) {
                swapchainSupportDetails.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapchainSupportDetails.formats.data());
            }
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
            if (presentModeCount != 0) {
                swapchainSupportDetails.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapchainSupportDetails.presentModes.data());
            }
            if (!swapchainSupportDetails.isAdequate()) {
                Log::Debug("\t\tHas no formats or present modes to display images in a swap chain");
                continue;
            }
            else {
                for (const auto& fmt : swapchainSupportDetails.formats) {
                    Log::Debug("\t\tSupports format: VkFormat[{}], colorspace: VkColorSpaceKHR[{}]", fmt.format, fmt.colorSpace);
                }
                for (const auto& mode : swapchainSupportDetails.presentModes) {
                    Log::Debug("\t\tSupports presentation modes: VkPresentModeKHR[{}]", mode);
                }
            }

            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                Log::Debug("\t\tIs not a GPU");
                continue;
            }

            if (!devFeaturesFunc(deviceFeatures)) {
                Log::Debug("\t\tDevice does not have all requested features.");
                continue;
            }
            else {
                Log::Debug("\t\tDevice has all requested features.");
            }

            // all requirements satisfied!
            physicalDevices.push_back(device);
            Log::Debug("Picked {}", deviceProperties.deviceName);
            break;
        }
        if (physicalDevices.empty()) {
            Log::Debug("Failed to find a suitable GPU with required queues!");
            exit(EXIT_FAILURE);
        }
	}

	// ---------------------

	PhysicalDevice::PhysicalDevice(const PhysicalDeviceBuilder& builder)
		: builder(builder) {
        Log::Debug("Selecting the first suitable PhysicalDevice...");
        handle = builder.physicalDevices[0];
	}
}