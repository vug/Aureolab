#include "VulkanRenderer.h"

#include "Core/Log.h"

#include <vulkan/vulkan.h>

#include <set>

VulkanRenderer::VulkanRenderer(Window& win) {
    // Window binding. Will make window to call OnResize when Window's framebuffer resized.
    win.SetUserPointer(this);

    Log::Debug("Constructing Vulkan Renderer...");
    Log::Debug("Creating Vulkan Instance...");
    // Vulkan Instance Parameters
    bool enableValidationLayers = true;
    VkDebugUtilsMessageSeverityFlagsEXT severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    std::vector<const char*> layers;
    // device features: deviceFeatures.samplerAnisotropy
    // any other queue families?

    // Check requested layers
    if (enableValidationLayers) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char* layerName : layers) {
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

    // Check required extensions
    uint32_t glfwExtensionCount = win.GetVulkanExtensionCount();
    const char** glfwExtensions = win.GetVulkanExtensions();
    Log::Debug("Extensions required by GLFW: {}", glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        Log::Debug("\t{}", glfwExtensions[i]);
    }
    // initialization of a vector from an array: v(a, a + len)
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Application Info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_2;
    // rest is not important
    appInfo.pApplicationName = "A Vulkan application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    // Debug setup for instance creation
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    // put logic into a function. Needed twice. -> or don't :-)
    debugCreateInfo = {};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    // If kept at warning throws harmless warnings such as "loaderAddLayerProperties: C:\VulkanSDK\1.2.189.2\Bin\VkLayer_device_simulation.json 
    // invalid layer manifest file version 1.2.0.  May cause errors."
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = DebugCallback;
    debugCreateInfo.pUserData = this;

    // Create Vulkan Instance
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        Log::Critical("failed to create Vulkan instance!");
        exit(EXIT_FAILURE);
    }

    // Create Surface
    if (win.CreateSurface(instance, &surface) != VK_SUCCESS) {
        Log::Critical("Cannot create Window Surface");
        exit(EXIT_FAILURE);
    }


    // Physical Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        Log::Critical("Failed to find GPUs with Vulkan support!");
        exit(EXIT_FAILURE);
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    Log::Debug("Available devices:");
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        Log::Debug("\t{}", properties.deviceName);
    }
    // choose first available and suitable device
    // (alternatively, we can rate device by their features first and choose best one)
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    Log::Debug("Suitable devices:");
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        Log::Debug("\tChecking suitability of {}", deviceProperties.deviceName);
        // Check required queues, store queue indices if there are any
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int ix = 0;
        for (const auto& queueFamily : queueFamilies) {
            // VK_QUEUE_TRANSFER_BIT is to copy/transfer buffers from CPU staging to GPU (actually implied by Graphics bit)
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = ix;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, ix, surface, &presentSupport);
            if (presentSupport) { indices.presentFamily = ix; }
            if (indices.isComplete()) { break; }
            ix++;
        }
        if (ix == queueFamilies.size()) {
            Log::Debug("\t\tDoes not have queues graphics and presentation.");
            continue;
        }
        // invariant: indices.isComplete() == true aka indices is a QueueFamilyIndices with required queues
        Log::Debug("\t\tHas {} queue families. Graphics index: {}, Present/Surface index: {}.",
            queueFamilyCount, indices.graphicsFamily.value(), indices.presentFamily.value());

        // Check required extensions
        const std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
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
            Log::Debug("\t\tHas required extensions: {}", s);
        }

        // Check Swapchain support
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;

            bool isAdequate() {
                return !formats.empty() && !presentModes.empty();
            }
        };
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }
        if (!details.isAdequate()) {
            Log::Debug("\t\tHas no formats or present modes to display images in a swap chain");
            continue;
        }
        else {
            for (const auto& fmt : details.formats) {
                Log::Debug("\t\tSupports format: VkFormat[{}], colorspace: VkColorSpaceKHR[{}]", fmt.format, fmt.colorSpace);
            }
            for (const auto& mode : details.presentModes) {
                Log::Debug("\t\tSupports presentation modes: VkPresentModeKHR[{}]", mode);
            }
        }

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            Log::Debug("\t\tIs not a GPU");
            continue;
        }
        
        if (!deviceFeatures.samplerAnisotropy) {
            Log::Debug("\t\tDoes not have Anisotrophic sampling feature.");
            continue;
        }
        else {
            Log::Debug("\t\tSupports Anisotrophic sampling feature!");
        }

        // all requirements satisfied!
        physicalDevice = device;
    }
    if (physicalDevice == VK_NULL_HANDLE) {
        Log::Debug("Failed to find a suitable GPU with required queues!");
        exit(EXIT_FAILURE);
    }
}

VulkanRenderer::~VulkanRenderer() {
    Log::Debug("Destructing Vulkan Renderer...");
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanRenderer::OnResize(int width, int height) {
    Log::Debug("Framebuffer resized: ({}, {})", width, height);
    // TODO: resize logic will come here
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::string msgType;
    switch (messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        msgType = "General";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        msgType = "Validation";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        msgType = "Performance";
        break;
    }

    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        Log::Trace("[ValidationLayer, {}] {}", msgType, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        Log::Info("[ValidationLayer, {}] {}", msgType, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        Log::Warning("[ValidationLayer, {}] {}", msgType, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        Log::Error("[ValidationLayer, {}] {}", msgType, pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}