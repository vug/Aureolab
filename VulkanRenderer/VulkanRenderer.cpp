#include "VulkanRenderer.h"

#include "Core/Log.h"

#include <vulkan/vulkan.h>

#include <fstream>
#include <set>

VulkanRenderer::VulkanRenderer(Window& win) {
    // Window binding. Will make window to call OnResize when Window's framebuffer resized.
    win.SetUserPointer(this);

    Log::Debug("Constructing Vulkan Renderer...");
    Log::Debug("Creating Vulkan Instance...");
    // Vulkan Instance Parameters
    bool enableValidationLayers = true;
    VkDebugUtilsMessageSeverityFlagsEXT severity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    std::vector<const char*> layers;
    // device features: deviceFeatures.samplerAnisotropy
    // any other queue families?
    // imageCount

    // Check requested layers
    {
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
    }

    // Create Vulkan Instance
    {
        // Application Info
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_API_VERSION_1_2;
        // rest is not important
        appInfo.pApplicationName = "A Vulkan application";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

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
        debugCreateInfo.pUserData = this;

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

        if (enableValidationLayers) {
            Log::Debug("Creating Debug Messenger...");
            // For debug messages not related to instance creation. (reusing previous VkDebugUtilsMessengerCreateInfoEXT)
            debugCreateInfo.messageSeverity = severity;
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(instance, &debugCreateInfo, nullptr, &debugMessenger);
            }
        }
    }

    // Create Surface
    if (win.CreateSurface(instance, &surface) != VK_SUCCESS) {
        Log::Critical("Cannot create Window Surface");
        exit(EXIT_FAILURE);
    }


    // Pick Physical Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    std::vector<VkPhysicalDevice> devices;

    // Available Devices
    {
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
    }


    // choose first available and suitable device
    // (alternatively, we can rate device by their features first and choose best one)
    Log::Debug("Suitable devices:");
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    QueueFamilyIndices indices;
    const std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        bool isAdequate() {
            return !formats.empty() && !presentModes.empty();
        }
    };
    SwapChainSupportDetails swapchainSupportDetails;
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


    // Create Logical Device
    Log::Debug("Creating Logical Device...");
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    {
        std::set<uint32_t> uniqueQueueFamilies =
        { indices.graphicsFamily.value(), indices.presentFamily.value() };

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

    {
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
            createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
            createInfo.ppEnabledLayerNames = layers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            Log::Debug("Failed to create logical device!");
        }
    }

    // Create Queues
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

    // Create Swapchain
    Log::Debug("Creating Swapchain...");
    surfaceFormat = swapchainSupportDetails.formats[0];
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

    const auto& capabilities = swapchainSupportDetails.capabilities;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swapExtent = capabilities.currentExtent;
        Log::Debug("\tSwapchain dimensions set to window size: ({}, {})", swapExtent.width, swapExtent.height);
    }
    else {
        int width, height;
        win.GetFramebufferSize(&width, &height);

        swapExtent = {
            static_cast<uint32_t>(width), static_cast<uint32_t>(height),
        };

        swapExtent.width = std::clamp(swapExtent.width,
            capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swapExtent.height = std::clamp(swapExtent.height,
            capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        Log::Debug("\tSwapchain dimensions chosen in min-max image extend range: ({}, {})", swapExtent.width, swapExtent.height);
    }


    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    Log::Debug("\tThere are {} images in the Swapchain. (min: {}, max: {})", 
        imageCount, capabilities.minImageCount, capabilities.maxImageCount);

    std::vector<VkImage> swapChainImages;
    {
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
        if (indices.graphicsFamily != indices.presentFamily) {
            // images can bu used by multiple queue families without explicit ownership transfer
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = std::array<uint32_t, 2>({ indices.graphicsFamily.value(), 
                                                                       indices.presentFamily.value(), }).data();
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

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            Log::Critical("failed to create swap chain!");
            exit(EXIT_FAILURE);
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
    }

    swapChainImageViews.resize(swapChainImages.size());
    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapChainImages[i];
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

        if (vkCreateImageView(device, &viewInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            Log::Critical("failed to create texture image view!");
            exit(EXIT_FAILURE);
        }
    }
}

VulkanRenderer::~VulkanRenderer() {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    Log::Debug("Destructing Vulkan Renderer...");
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {  // TODO: also if enableValidationLayers
        func(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanRenderer::OnResize(int width, int height) {
    Log::Debug("Framebuffer resized: ({}, {})", width, height);
    // TODO: resize logic will come here
}

VkPipeline VulkanRenderer::CreateExampleGraphicsPipeline(const std::string& vertFilename, const std::string& fragFilename) {
    Log::Debug("Creating Graphics Pipeline...");
    // optional parameters: shaders, Vertex class with binding and attribute descriptions,
    // VkPrimitiveTopology topology, VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace
    // VkSampleCountFlagBits rasterizationSamples, VkPipelineColorBlendAttachmentState alpha blend settings

    Log::Debug("\tCreating Shader Modules and Shader Stage Info...");
    auto vertShaderByteCode = ReadFile(vertFilename);
    auto fragShaderByteCode = ReadFile(fragFilename);
    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderByteCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderByteCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    // Entrypoint. can combine multiple shaders into a single file, each of which having a different entrypoint.
    vertShaderStageInfo.pName = "main";
    // To set shader constants to help compiler (more efficient than doing it at render time)
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    Log::Debug("\tVertex Input State Info...");
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
    // TODO: Vertex descriptions will come later, first example does not use vertex input
    //auto bindingDescription = Vertex::getBindingDescription();
    //auto attributeDescriptions = Vertex::getAttributeDescriptions();
    //vertexInputInfo.vertexBindingDescriptionCount = 1;
    //vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    //vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    //vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    // TODO: Have a BaseVertex class with getBindingDescription() and getAttributeDescriptions() virtual methods. Graphics Pipeline can take one.
    // TODO: Or, it can be templated, and call static functions for binding description?

    Log::Debug("\tInput Assembly State Info...");
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // There are also POINT_LIST, LINE_LIST, TRIANGLE_STRIP etc.
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // restart is useful in instance rendering
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    Log::Debug("\tNo Tesselation State Info.");

    Log::Debug("\tViewport State Info...");
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapExtent.width;
    viewport.height = (float)swapExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapExtent;
    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor;

    Log::Debug("\tRasterization State Info...");
    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // when true clamps fragments nearer than near plane or further than far plane
    // as opposed to discarding (might be useful for shadow maps)
    rasterizerInfo.depthClampEnable = VK_FALSE;
    // when tru geometry never passes rasterizer stage. => disables framebuffer.
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    // Triangle rendering mode. {FILL, LINE, POINT}. Other modes require a GPU feature.
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    // NONE, FRONT, BACK, or FRONT_AND_BACK
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    // or clock-wise (we did y-flip, hence need to switch from CW to CCW)
    rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    // contant depth added to each fragment
    rasterizerInfo.depthBiasConstantFactor = 0.0f;
    rasterizerInfo.depthBiasClamp = 0.0f;
    rasterizerInfo.depthBiasSlopeFactor = 0.0f;
    // max value depends on hardward. lw > 1 requires wideLines GPU feature.
    rasterizerInfo.lineWidth = 1.0f;

    Log::Debug("\tMultisample State Info...");
    VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    // number of samples per fragment
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.minSampleShading = 1.0f;
    multisamplingInfo.pSampleMask = nullptr;
    // Might be useful later.
    multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingInfo.alphaToOneEnable = VK_FALSE;

    // TODO: add depth and stencil later
    Log::Debug("\tDepth Stencil State Info...");
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    //depthStencilInfo.front. ...

    Log::Debug("\tColor Blend State Info...");
    // About combining fragment shader color with the color already in Framebuffer
    // Can be done via colorBlendOp
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // When false, new color from fragment shader is written into Framebuffer without modification. 
    colorBlendAttachment.blendEnable = VK_FALSE;

    // Example
    VkPipelineColorBlendAttachmentState standardAlphaBlending{};
    standardAlphaBlending.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    standardAlphaBlending.blendEnable = VK_TRUE;
    standardAlphaBlending.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    standardAlphaBlending.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    standardAlphaBlending.colorBlendOp = VK_BLEND_OP_ADD;
    standardAlphaBlending.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    standardAlphaBlending.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    standardAlphaBlending.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
    colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // Blending also can be done via bitwise operations. Turning this one disables color blending, as if blendEnable was false
    colorBlendingInfo.logicOpEnable = VK_FALSE;
    //colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlendingInfo.attachmentCount = 1;
    colorBlendingInfo.pAttachments = &colorBlendAttachment;

    // No need for a VkPipelineDynamicStateCreateInfo at the moment.
    // Not changing any pipeline settings dynamically at the moment.

    Log::Debug("\tCreating Pipeline Layout...");
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    // TODO: will come when loading vertex data
    //pipelineLayoutInfo.setLayoutCount = 1;
    //pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    //pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    //pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        Log::Critical("failed to create pipeline layout!");
        exit(EXIT_FAILURE);
    }

    Log::Debug("\tRender Pass...");
    /*
     A Renderpass is a collection of N attachments, M subpassesand their L dependencies.
     Describes how attachments are used over the course of subpasses.
     Each subpass can use N_m <= N subset of attachments as their input, color, depth, resolve etc. attachment
     Dependencies are memory-deps between src and dst subpasses. Self-dependencies are possible.
    */
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = surfaceFormat.format;
    // should this be the same as pipeline rasterizationSamples?
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // at the beginning of subpass: _LOAD would preserved what was previously there. Other: _DONT_CARE
    // for depth/stencil attachment only apply to depth
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // at the end of subpass: or _DONT_CARE
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // ignored when dealing with a color attachment, used by depth/stencil attachments
    // currently, not using stencil buffer
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Layout at the beginning of render pass. Don't care about the layout before rendering. Will overwrite anyway.
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // Layout to automatically transition to after render pass finishes
    // Since this is a single render-pass sub-pass example, this attachment will be displayed after drawn
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Only one sub-pass at the moment
    VkAttachmentReference colorAttachmentRef{};
    // index in Renderpass' VkAttachmentDescription* pAttachments
    colorAttachmentRef.attachment = 0;
    // Vulkan will transition the attachment into this layout when subpass will start
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // not _COMPUTE
    subpass.colorAttachmentCount = 1;
    // index in this "array" is directly referenced in fragment shader, say `layout(location = 0) out vec4 outColor`
    subpass.pColorAttachments = &colorAttachmentRef;

    // specify memory and execution dependencies between subpasses
    // We want our only single subpass to happen after swap chain image is acquired
    // Option 1: set waitStages for the imageAvailableSemaphore to VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT <- render pass does not start until image is available
    // Option 2: (this one) make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // special value reference: "subpass before renderpass"
    dependency.dstSubpass = 0; // first and only subpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Renderpass is created by providing attachments, subpasses that use them, and the dependency relationship between subpasses
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        Log::Critical("failed to create render pass!");
        exit(EXIT_FAILURE);
    }
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    // TODO: return correct pipeline
    return VkPipeline();
}

std::vector<char> VulkanRenderer::ReadFile(const std::string& filename) {
    // start reading at the end of the file. read as binary.
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        Log::Critical("failed to open shader file!");
        exit(EXIT_FAILURE);
    }

    size_t fileSize = (size_t)file.tellg();
    Log::Debug("{} has {} characters.", filename, fileSize);
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
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