#include "VulkanRenderer.h"

#include "Core/Log.h"

#include <vulkan/vulkan.h>

VulkanRenderer::VulkanRenderer(Window& win) {
    // Window binding. Will make window to call OnResize when Window's framebuffer resized.
    win.SetUserPointer(this);

    Log::Debug("Constructing Vulkan Renderer...");
    Log::Debug("Creating Vulkan Instance...");
    // Vulkan Instance Parameters
    bool enableValidationLayers = true;
    VkDebugUtilsMessageSeverityFlagsEXT severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
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