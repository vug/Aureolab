#include "VulkanContext.h"

#include "Core/Log.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <cassert>
#include <functional>
#include <set>
#include <string>

VulkanContext::VulkanContext(VulkanWindow& win, bool validation) {
    /* Potential configuration options:
    - Device features such as samplerAnisotropy
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

    std::vector<const char*> vulkanLayers;
    std::tie(instance, debugMessenger, vulkanLayers) = CreateInstance(windowExtensionCount, windowExtensions, validation);
    shouldDestroyDebugUtils = validation;
    surface = CreateSurface(win, instance);
    VkPhysicalDevice physicalDevice;
    QueueFamilyIndices queueIndices;
    SwapChainSupportDetails swapchainSupportDetails;
    std::vector<const char*> requiredExtensions;
    std::tie(physicalDevice, queueIndices, swapchainSupportDetails, requiredExtensions) = CreatePhysicalDevice(instance, surface);
    std::tie(device, graphicsQueue, presentQueue) = CreateLogicalDevice(physicalDevice, queueIndices, requiredExtensions, validation, vulkanLayers);
    std::tie(swapchain, swapchainInfo.surfaceFormat, swapchainInfo.extent, swapchainInfo.imageViews) = CreateSwapChain(device, surface, queueIndices, swapchainSupportDetails);
    commandPool = CreateGraphicsCommandPool(device, queueIndices.graphicsFamily.value());

    // Initialize synchronization objects
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // otherwise we'll wait for the fence to signal for the first frame eternally
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    assert(vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence) == VK_SUCCESS);

    //for the semaphores we don't need any flags
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.flags = 0;
    assert(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore) == VK_SUCCESS);
    assert(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore) == VK_SUCCESS);

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
}

VulkanContext::~VulkanContext() {
    Log::Debug("Destructing Vulkan Context...");
    vkDestroyFence(device, renderFence, nullptr);
    vkDestroySemaphore(device, presentSemaphore, nullptr);
    vkDestroySemaphore(device, renderSemaphore, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    for (auto imageView : swapchainInfo.imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    if (shouldDestroyDebugUtils) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, nullptr);
        }
    }
    vkDestroyInstance(instance, nullptr);
}

std::tuple<VkInstance&, VkDebugUtilsMessengerEXT&, std::vector<const char*>&> VulkanContext::CreateInstance(uint32_t requestedExtensionCount, const char** requestedExtensions, bool enableValidationLayers) {
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

    VkDebugUtilsMessengerEXT debugMessenger;
    if (enableValidationLayers) {
        Log::Debug("Creating Debug Messenger...");
        // For debug messages not related to instance creation. (reusing previous VkDebugUtilsMessengerCreateInfoEXT)
        debugCreateInfo.messageSeverity = severity;
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, &debugCreateInfo, nullptr, &debugMessenger);
        }
    }
    return { instance, debugMessenger, layers };
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

std::tuple<VkPhysicalDevice&, QueueFamilyIndices, SwapChainSupportDetails, std::vector<const char*>> VulkanContext::CreatePhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface) {
    /* Potential Configuration Options:
    - Queues to request (to get a compute queue)
    - Device features to query
    */
    Log::Debug("Creating Physical Device...");
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    std::vector<VkPhysicalDevice> devices;

    // Available Devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        Log::Critical("Failed to find GPUs with Vulkan support!");
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

    // choose first available and suitable device
    // (alternatively, we can rate devices by their feature sets and choose best one)
    Log::Debug("Suitable devices:");
    QueueFamilyIndices indices;

    SwapChainSupportDetails swapchainSupportDetails;

    std::vector<const char*> requiredExtensions;
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
        requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
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

        if (!deviceFeatures.samplerAnisotropy) {
            Log::Debug("\t\tDoes not have Anisotrophic sampling feature.");
            continue;
        }
        else {
            Log::Debug("\t\tSupports Anisotrophic sampling feature.");
        }

        // all requirements satisfied!
        physicalDevice = device;
        Log::Debug("Picked {}", deviceProperties.deviceName);
        break;
    }
    if (physicalDevice == VK_NULL_HANDLE) {
        Log::Debug("Failed to find a suitable GPU with required queues!");
        exit(EXIT_FAILURE);
    }
    return { physicalDevice, indices, swapchainSupportDetails, requiredExtensions };
}

std::tuple<VkDevice&, VkQueue, VkQueue> VulkanContext::CreateLogicalDevice(VkPhysicalDevice& physicalDevice, QueueFamilyIndices& queueIndices, std::vector<const char*>& requiredExtensions, bool enableValidationLayers, std::vector<const char*>& vulkanLayers) {
    Log::Debug("Creating Logical Device...");
    VkDevice device;
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    {
        std::set<uint32_t> uniqueQueueFamilies =
        { queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value() };

        float queuePriority = 1.0f; // determine scheduling of command buffer execution
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
    }

    // Device features to be used / enabled.
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE; // use anisotropy filters for textures

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(vulkanLayers.size());
        createInfo.ppEnabledLayerNames = vulkanLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        Log::Debug("Failed to create logical device!");
    }

    VkQueue graphicsQueue, presentQueue;
    vkGetDeviceQueue(device, queueIndices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueIndices.presentFamily.value(), 0, &presentQueue);

    return { device, graphicsQueue, presentQueue };
}

std::tuple<VkSwapchainKHR&, VkSurfaceFormatKHR&, VkExtent2D&, std::vector<VkImageView>> VulkanContext::CreateSwapChain(VkDevice& device, VkSurfaceKHR& surface, QueueFamilyIndices& queueIndices, SwapChainSupportDetails& swapchainSupportDetails) {
    Log::Debug("Creating Swapchain...");
    VkSwapchainKHR swapchain;

    VkSurfaceFormatKHR surfaceFormat = swapchainSupportDetails.formats[0];
    for (const auto& availableFormat : swapchainSupportDetails.formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = availableFormat;
        }
    }
    Log::Debug("\tChosen surface format - format: {}, colorspace: {}", surfaceFormat.format, surfaceFormat.colorSpace);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& availablePresentMode : swapchainSupportDetails.presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = availablePresentMode;
        }
    }
    Log::Debug("\tChosen present mode: {}", presentMode);

    VkExtent2D swapExtent;
    const auto& capabilities = swapchainSupportDetails.capabilities;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swapExtent = capabilities.currentExtent;
        Log::Debug("\tSwapchain dimensions set to window size: ({}, {})", swapExtent.width, swapExtent.height);
    }
    else {
        int width = 800, height = 600; // default value, but better to bring it from window
        //win.GetFramebufferSize(&width, &height);

        swapExtent = {
            static_cast<uint32_t>(width), static_cast<uint32_t>(height),
        };

        swapExtent.width = std::clamp(swapExtent.width,
            capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swapExtent.height = std::clamp(swapExtent.height,
            capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        Log::Debug("\tSwapchain dimensions chosen in min-max image extend range: ({}, {})", swapExtent.width, swapExtent.height);
    }

    // TODO: make this a parameter (to compare performance of swapchains of sizes 2/3/4 etc.)
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    Log::Debug("\tThere are {} images in the Swapchain. (min: {}, max: {})",
        imageCount, capabilities.minImageCount, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapExtent;
    // Can be 2 for stereoscopic 3D/VR applications
    createInfo.imageArrayLayers = 1;
    // We'll render directly into this image, therefor they'll be color attachments
    // We could have rendered scenes into separate images first to do post-processing on them
    // and then copy/transfer them to a swap chain image via VK_IMAGE_USAGE_TRANSFER_DST_BIT
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Wheter the same queue handles drawing and presentation or not
    if (queueIndices.graphicsFamily != queueIndices.presentFamily) {
        // images can bu used by multiple queue families without explicit ownership transfer
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = std::array<uint32_t, 2>({ queueIndices.graphicsFamily.value(),
                                                                    queueIndices.presentFamily.value(), }).data();
    }
    else {
        // at any given time image is owned only by one queue family, ownership must be explicitly transferred to other family before use
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    Log::Debug("\tImage Sharing Mode (among queue families): {}", createInfo.imageSharingMode);

    // an example transform could be horizontal flip
    createInfo.preTransform = swapchainSupportDetails.capabilities.currentTransform;
    // ignore alpha, no blending with other windows in the window system.
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    // don't care about obscured pixels (behind another window)
    createInfo.clipped = VK_TRUE;
    // Swapchain becomes invalid on resize, and old swap chain is referred while creating new one. Here were are creating swap chain the first time.
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        Log::Critical("failed to create swap chain!");
        exit(EXIT_FAILURE);
    }

    std::vector<VkImage> swapchainImages;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    std::vector<VkImageView> swapchainImageViews;
    swapchainImageViews.resize(swapchainImages.size());
    for (uint32_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages[i];
        // can also be 1D, 3D or cube map
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = surfaceFormat.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        // VR app can have 2 layers, one of each eye
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
            Log::Critical("failed to create texture image view!");
            exit(EXIT_FAILURE);
        }
    }

    //std::vector<std::reference_wrapper<VkImageView>> ret(swapchainImages.begin(), swapchainImages.end());
    return { swapchain, surfaceFormat, swapExtent, swapchainImageViews };
}

VkCommandPool& VulkanContext::CreateGraphicsCommandPool(VkDevice& device, uint32_t graphicsQueueFamilyIndex) {
    Log::Debug("Creating Command Pool...");
    VkCommandPool commandPool;

    // Rendering draw commands will go to graphics queue
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    // Expect to reset buffers spawned from this pool.
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        Log::Debug("failed to create command pool!");
        exit(EXIT_FAILURE);
    }

    return commandPool;
}

VkFramebuffer& VulkanContext::CreateFramebuffer(const VkDevice& device, const VkRenderPass& renderPass, const VkImageView& imageView, const VkExtent2D& extent) {
    Log::Debug("Creating Framebuffer...");

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    // can only be used with compatible (same number and type of attachments) render passes
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    VkImageView attachments[] = { imageView };
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    VkFramebuffer framebuffer;
    assert(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer) == VK_SUCCESS);
    return framebuffer;
}

std::vector<VkFramebuffer> VulkanContext::CreateSwapChainFrameBuffers(const VkDevice& device, const VkRenderPass& renderPass, const SwapchainInfo& swapchainInfo) {
    const auto& presentImageViews = swapchainInfo.imageViews;
    std::vector<VkFramebuffer> presentFramebuffers(presentImageViews.size());
    for (size_t i = 0; i < presentImageViews.size(); i++) {
        presentFramebuffers[i] = CreateFramebuffer(device, renderPass, presentImageViews[i], swapchainInfo.extent);
    }
    return presentFramebuffers;
}

void VulkanContext::OnResize(int width, int height) {
    Log::Debug("Framebuffer resized: ({}, {})", width, height);
    // TODO: resize logic will come here
}

void VulkanContext::drawFrameBlocked(VkRenderPass& renderPass, VkCommandBuffer& cmdBuf, const std::vector<VkFramebuffer>& swapchainFramebuffers, const SwapchainInfo& swapchainInfo, const VkClearValue& clearValue, std::function<void(VkCommandBuffer&)> cmdFunc) {
    // Vulkan executes commands asynchroniously/independently. 
    // Need explicit dependency declaration for correct order of execution, i.e. synchronization
    // use fences to sync main app with command queue ops, use semaphors to sync operations within/across command queues

    // wait until the GPU has finished rendering the last frame. Timeout of 1 second
    assert(vkWaitForFences(device, 1, &renderFence, true, 1000000000) == VK_SUCCESS); // 1sec = 1000000000
    assert(vkResetFences(device, 1, &renderFence) == VK_SUCCESS);

    uint32_t swapchainImageIndex;
    // Request an imaage and wait until it's acquired (or timeout)
    VkResult result = vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // recreateSwapChain();
        assert(false);
        // TODO implement resize
    } else { 
        assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
    }

    // previous run of cmdBuf was finished, safely reset it to begin the recording again
    assert(vkResetCommandBuffer(cmdBuf, 0) == VK_SUCCESS);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // We'll use this buffer only once per frame
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    assert(vkBeginCommandBuffer(cmdBuf, &cmdBeginInfo) == VK_SUCCESS);

    VkRenderPassBeginInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = renderPass;
    // using render area we can render a smaller part of a bigger image
    rpInfo.renderArea.offset = { 0, 0 };
    rpInfo.renderArea.extent = swapchainInfo.extent;
    rpInfo.framebuffer = swapchainFramebuffers[swapchainImageIndex];
    rpInfo.clearValueCount = 1;
    rpInfo.pClearValues = &clearValue;
    // Will bind the Framebuffer, clear the Image, put the Image in the layout specified at RenderPass creation
    vkCmdBeginRenderPass(cmdBuf, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Populate command buffer with the calls provided in the function
    cmdFunc(cmdBuf);

    // finishes rendering and transition image to "ready to be displayed" state that we specified
    vkCmdEndRenderPass(cmdBuf);
    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    assert(vkEndCommandBuffer(cmdBuf) == VK_SUCCESS);

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit.pWaitDstStageMask = &waitStage;
    submit.waitSemaphoreCount = 1;
    // Queue will wait until when the Swapchain is ready
    submit.pWaitSemaphores = &presentSemaphore;
    submit.signalSemaphoreCount = 1;
    // When Queue processing is done on GPU it'll signal that rendering has finished
    submit.pSignalSemaphores = &renderSemaphore;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmdBuf;
    // CPU submission will wait for GPU rendering to complete
    assert(vkQueueSubmit(graphicsQueue, 1, &submit, renderFence) == VK_SUCCESS);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;
    // Present after GPU rendering is completed
    presentInfo.pWaitSemaphores = &renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;
    assert(vkQueuePresentKHR(graphicsQueue, &presentInfo) == VK_SUCCESS);
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
