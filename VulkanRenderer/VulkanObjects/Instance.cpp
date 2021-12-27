#include "Instance.h"

#include "Core/Log.h"

namespace vr {
    InstanceBuilder::InstanceBuilder(const Params& params)
        : params(params),
        layers(initLayers(params)),
        extensions(initExtensions(params)),
        appInfo(initAppInfo(params)),
        info(initInfo(params)) {}

    InstanceBuilder::operator const VkInstanceCreateInfo* () {
        return &info;
    }

    std::vector<const char*> InstanceBuilder::initLayers(const Params& params) {
        std::vector<const char*> newLayers;

        // Add Vulkan Validation Layer if debug turned on
        if (params.validation) {
            const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
            newLayers.insert(newLayers.end(), validationLayers.begin(), validationLayers.end());
        }

        // Add layers that were requested explicitly
        for (auto& reqLayer : params.requestedLayers) {
            if (std::find(newLayers.begin(), newLayers.end(), reqLayer) == newLayers.end())
                newLayers.push_back(reqLayer); // hopefully copy
        }

        // Check whether requested layers are available
        uint32_t numAvailableLayers;
        vkEnumerateInstanceLayerProperties(&numAvailableLayers, nullptr);
        std::vector<VkLayerProperties> availableLayers(numAvailableLayers);
        vkEnumerateInstanceLayerProperties(&numAvailableLayers, availableLayers.data());
        for (const char* layerName : newLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                Log::Critical("Requested Vulkan layer {} not supported or found!", layerName);
                exit(EXIT_FAILURE);
            }
        }

        return newLayers;
    }

    std::vector<const char*> InstanceBuilder::initExtensions(const Params& params) {
        std::vector<const char*> newExtensions;

        if (params.validation) {
            newExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        if (!params.headless) {
            const std::vector<const char*> surfaceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface" };
            newExtensions.insert(newExtensions.end(), surfaceExtensions.begin(), surfaceExtensions.end());
        }

        for (auto& reqExt : params.requestedExtensions) {
            if (std::find(newExtensions.begin(), newExtensions.end(), reqExt) == newExtensions.end())
                newExtensions.push_back(reqExt); // hopefully copy
        }

        return newExtensions;
    }

    VkApplicationInfo InstanceBuilder::initAppInfo(const Params& params) {
        return {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            "Vulkan Application",
            1,
            "Vulkan Engine",
            1,
            VK_API_VERSION_1_2,
        };
    }

    VkInstanceCreateInfo InstanceBuilder::initInfo(const Params& params) {
        // Debug setup for only instance creation/destruction
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        // If kept at warning throws harmless warnings such as "loaderAddLayerProperties: C:\VulkanSDK\1.2.189.2\Bin\VkLayer_device_simulation.json 
        // invalid layer manifest file version 1.2.0. May cause errors." Therefore only keeping at error level.
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT,
            VkDebugUtilsMessageTypeFlagsEXT,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*) {
                Log::Error("Vulkan Instance Creation/Destruction Error: {}", pCallbackData->pMessage);
                return VK_FALSE;
        };

        return {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            params.validation ? &debugCreateInfo : nullptr,
            0,
            &appInfo,
            static_cast<uint32_t>(layers.size()),
            layers.data(),
            static_cast<uint32_t>(extensions.size()),
            extensions.data(),
        };
    }

    // --------------------------

    Instance::Instance(const InstanceBuilder& builder) {
        Log::Debug("Creating Instance...");
        VkResult result = vkCreateInstance(&builder.info, nullptr, &handle);
        if (result != VK_SUCCESS) {
            Log::Critical("Failed to create Vulkan Instance!");
            exit(EXIT_FAILURE);
        }
    }

    Instance::~Instance() {
        Log::Debug("Destroying Instance...");
        vkDestroyInstance(handle, nullptr);
    }
}