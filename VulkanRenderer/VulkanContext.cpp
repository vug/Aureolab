#include "VulkanContext.h"
#include "VulkanContext.h"

#include "Core/Log.h"

#include <vector>

VulkanContext::VulkanContext(VulkanWindow& win, bool validation) {
    /* Potential configuration options:
    - Device features such as samplerAnisotropy
    - Queues to request (to get a compute queue)
    - Number of images in SwapChain
    */
    Log::Debug("Constructing Vulkan Renderer...");

    // Window binding. Will make the window to call OnResize when Window's framebuffer resized.
    win.SetUserPointer(this);
    uint32_t windowExtensionCount = win.GetVulkanExtensionCount();
    const char** windowExtensions = win.GetVulkanExtensions();
    Log::Debug("Extensions required by the windowing system: {}", windowExtensionCount);
    for (uint32_t i = 0; i < windowExtensionCount; i++) {
        Log::Debug("\t{}", windowExtensions[i]);
    }

    instance = CreateInstance(windowExtensionCount, windowExtensions, validation, debugMessenger);
    shouldDestroyDebugUtils = validation;
    surface = CreateSurface(win, instance);
}

VulkanContext::~VulkanContext() {
    Log::Debug("Destructing Vulkan Context...");
    vkDestroySurfaceKHR(instance, surface, nullptr);
    if (shouldDestroyDebugUtils) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, nullptr);
        }
    }
    vkDestroyInstance(instance, nullptr);
}

VkInstance& VulkanContext::CreateInstance(uint32_t requestedExtensionCount, const char** requestedExtensions, bool enableValidationLayers, VkDebugUtilsMessengerEXT& outDebugMessenger) {
    /* Potential Configuration Options:
    - Whole VkApplicationInfo (API version, App Name/Version etc)
    - Debug
      - Callback Function
      - Severity Flags
      - User Data Pointer
    - Validation: Enable/Disable Features and Checks
    - Headless context
    */
    Log::Debug("Creating Vulkan Instance...");
    VkInstance instance;

    VkDebugUtilsMessageSeverityFlagsEXT severity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    std::vector<const char*> layers;
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

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_2;
    // rest is not important
    appInfo.pApplicationName = "A Vulkan application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    // initialization of a vector from an array: v(a, a + len)
    std::vector<const char*> extensions(requestedExtensions, requestedExtensions + requestedExtensionCount);
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Debug setup for instance creation/destruction
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

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        // For debug messages related to instance creation
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

    if (enableValidationLayers) {
        Log::Debug("Creating Debug Messenger...");
        // For debug messages not related to instance creation. (reusing previous VkDebugUtilsMessengerCreateInfoEXT)
        debugCreateInfo.messageSeverity = severity;
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, &debugCreateInfo, nullptr, &outDebugMessenger);
        }
    }
    return instance;
}

VkSurfaceKHR& VulkanContext::CreateSurface(VulkanWindow& win, VkInstance& instance) {
    Log::Debug("Creating Surface...");
    VkSurfaceKHR surface;
    if (win.CreateSurface(instance, &surface) != VK_SUCCESS) {
        Log::Critical("Failed to create Window Surface!");
        exit(EXIT_FAILURE);
    }
    return surface;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
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
