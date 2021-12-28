#include "Swapchain.h"

#include "Core/Log.h"

#include <array>

namespace vr {
	SwapchainBuilder::SwapchainBuilder(const Device& device) 
		: device(device) {

        const PhysicalDevice& physicalDevice = device.builder.physicalDevice;
        const Surface& surface = physicalDevice.surface;
        const SwapChainSupportDetails swapchainSupportDetails = physicalDevice.builder.swapchainSupportDetails;
        const QueueFamilyIndices& queueIndices = physicalDevice.builder.indices;
        const VulkanWindow& win = surface.win;

        surfaceFormat = swapchainSupportDetails.formats[0];
        for (const auto& availableFormat : swapchainSupportDetails.formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = availableFormat;
            }
        }
        Log::Debug("\tChosen surface format - format: {}, colorspace: {}", surfaceFormat.format, surfaceFormat.colorSpace);

        presentMode = VK_PRESENT_MODE_FIFO_KHR;
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

        // TODO: make this a parameter (to compare performance of swapchains of sizes 2/3/4 etc.)
        imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }
        Log::Debug("\tThere are {} images in the Swapchain. (min: {}, max: {})",
            imageCount, capabilities.minImageCount, capabilities.maxImageCount);

        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = surface;
        info.minImageCount = imageCount;
        info.imageFormat = surfaceFormat.format;
        info.imageColorSpace = surfaceFormat.colorSpace;
        info.imageExtent = swapExtent;
        // Can be 2 for stereoscopic 3D/VR applications
        info.imageArrayLayers = 1;
        // We'll render directly into this image, therefore they'll be color attachments.
        // Alternatively, we could have rendered scenes into separate images first to do post-processing on them
        // and then copy/transfer them to a swap chain image via VK_IMAGE_USAGE_TRANSFER_DST_BIT
        // TODO: have a post-processing parameter
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // Wheter the same queue handles drawing and presentation or not
        if (queueIndices.graphicsFamily != queueIndices.presentFamily) {
            // images can bu used by multiple queue families without explicit ownership transfer
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = std::array<uint32_t, 2>({ queueIndices.graphicsFamily.value(),
                                                                        queueIndices.presentFamily.value(), }).data();
        }
        else {
            // at any given time image is owned only by one queue family, ownership must be explicitly transferred to other family before use
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = nullptr;
        }
        Log::Debug("\tImage Sharing Mode (among queue families): {}", info.imageSharingMode);

        // an example transform could be horizontal flip
        info.preTransform = swapchainSupportDetails.capabilities.currentTransform;
        // ignore alpha, no blending with other windows in the window system.
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMode;
        // don't care about obscured pixels (behind another window)
        info.clipped = VK_TRUE;
        // Swapchain becomes invalid on resize, and old swap chain is referred while creating new one. Here were are creating swap chain the first time.
        info.oldSwapchain = VK_NULL_HANDLE;

        // 
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = VK_NULL_HANDLE;
        // can also be 1D, 3D or cube map
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = surfaceFormat.format;
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        // VR app can have 2 layers, one of each eye
        imageViewInfo.subresourceRange.layerCount = 1;
	}

    // -----------------------------

    Swapchain::Swapchain(const SwapchainBuilder& builder)
        : builder(builder) {
        Log::Debug("Creating Swapchain...");
        const Device& device = builder.device;

        VkResult result = vkCreateSwapchainKHR(device, &builder.info, nullptr, &handle);
        if (result != VK_SUCCESS) {
            Log::Critical("Failed to create swap chain!");
            exit(EXIT_FAILURE);
        }

        Log::Debug("Acquiring Swapchain Images...");
        uint32_t imCnt;
        vkGetSwapchainImagesKHR(device, handle, &imCnt, nullptr);
        imageHandles.resize(imCnt);
        vkGetSwapchainImagesKHR(device, handle, &imCnt, imageHandles.data());

        Log::Debug("Creating Swapchain ImageViews...");
        swapchainInfo.imageViews.resize(imageHandles.size());
        for (uint32_t i = 0; i < imageHandles.size(); i++) {
            VkImageViewCreateInfo viewInfo = builder.imageViewInfo;
            viewInfo.image = imageHandles[i];

            VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &swapchainInfo.imageViews[i]);
            if (result != VK_SUCCESS) {
                Log::Critical("Failed to create Swapchain Image View!");
                exit(EXIT_FAILURE);
            }
        }

        swapchainInfo.surfaceFormat = builder.surfaceFormat;
        swapchainInfo.extent = builder.swapExtent;
        swapchainInfo.depthFormat = VK_FORMAT_D32_SFLOAT;
    }
    
    Swapchain::~Swapchain() {
        Log::Debug("Creating Swapchain Image Views");
        for (auto& imageView : swapchainInfo.imageViews)
            vkDestroyImageView(builder.device, imageView, nullptr);
        Log::Debug("Destroying Swapchain...");
        vkDestroySwapchainKHR(builder.device, handle, nullptr);
    }
}