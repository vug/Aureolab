#include "VulkanContext.h"

#include "Core/Log.h"

#include <cassert>
#include <functional>
#include <set>
#include <string>

ImmediateCommandSubmitter::ImmediateCommandSubmitter(const VkDevice& device, const VkQueue& graphicsQueue, const uint32_t graphicsQueueFamilyIndex, VulkanDestroyer& destroyer)
    : device(device), queue(graphicsQueue) {
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // VK_FENCE_CREATE_SIGNALED_BIT flag is not needed because we won't wait for it before sending commands
    assert(vkCreateFence(device, &fenceCreateInfo, nullptr, &uploadFence) == VK_SUCCESS);
    destroyer.Add(uploadFence);

    cmdPool = VulkanContext::CreateGraphicsCommandPool(device, graphicsQueueFamilyIndex);
    destroyer.Add(cmdPool);
}

void ImmediateCommandSubmitter::Submit(std::function<void(VkCommandBuffer cmd)>&& function) {
    VkCommandBuffer cmdBuf = VulkanContext::CreateCommandBuffer(device, cmdPool);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // We'll use this buffer only once for uploading a mesh and be done with it
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    assert(vkBeginCommandBuffer(cmdBuf, &cmdBeginInfo) == VK_SUCCESS);
    function(cmdBuf);
    assert(vkEndCommandBuffer(cmdBuf) == VK_SUCCESS);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;
    submitInfo.signalSemaphoreCount = 0;
    assert(vkQueueSubmit(queue, 1, &submitInfo, uploadFence) == VK_SUCCESS);

    vkWaitForFences(device, 1, &uploadFence, true, 9999999999);
    vkResetFences(device, 1, &uploadFence);
    vkResetCommandPool(device, cmdPool, 0);
}

VulkanContext::VulkanContext(VulkanWindow& win) {
    /* Potential configuration options:
    - Device features such as samplerAnisotropy
    - Number of images in SwapChain
    */
    Log::Debug("Constructing Vulkan Context...");

    // Window binding. Will make the window to call OnResize when Window's framebuffer resized.
    win.SetUserPointer(this);

    instance = std::make_unique<vr::Instance>(vr::InstanceBuilder());

    if (instance->builder.params.validation)
        debugMessenger = std::make_unique<vr::DebugMessenger>(vr::DebugMessengerBuilder(*instance));

    // TODO: should depend on instance->builder.params.headless
    surface = std::make_unique<vr::Surface>(vr::SurfaceBuilder(*instance, win));

    physicalDevice = std::make_unique<vr::PhysicalDevice>(vr::PhysicalDeviceBuilder(*instance, *surface));

    device = std::make_unique<vr::Device>(vr::DeviceBuilder(*physicalDevice));

    allocator = std::make_unique<vr::Allocator>(vr::AllocatorBuilder(*device));

    destroyer = std::make_unique<VulkanDestroyer>(*device, *allocator);

    swapchain = std::make_unique<vr::Swapchain>(vr::SwapchainBuilder(*device));
}

VulkanContext::~VulkanContext() {
    Log::Debug("Destructing Vulkan Context...");
    destroyer->DestroyAll();
}


VkCommandPool VulkanContext::CreateGraphicsCommandPool(const VkDevice& device, uint32_t graphicsQueueFamilyIndex) {
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

FrameSyncCmd VulkanContext::CreateFrameSyncCmd(const VkDevice& device, uint32_t graphicsQueueFamilyIndex) {
    FrameSyncCmd frame = {};
    frame.commandPool = CreateGraphicsCommandPool(device, graphicsQueueFamilyIndex);
    frame.mainCommandBuffer = CreateCommandBuffer(device, frame.commandPool);

    // Initialize synchronization objects
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // otherwise we'll wait for the fence to signal for the first frame eternally
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    assert(vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.renderFence) == VK_SUCCESS);

    //for the semaphores we don't need any flags
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.flags = 0;
    assert(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.presentSemaphore) == VK_SUCCESS);
    assert(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.renderSemaphore) == VK_SUCCESS);

    return frame;
}

VkCommandBuffer VulkanContext::CreateCommandBuffer(const VkDevice& device, const VkCommandPool& cmdPool, VkCommandBufferLevel level) {
    Log::Debug("Creating Graphics Command Buffer...");
    VkCommandBuffer cmdBuf;
    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = cmdPool;
    cmdAllocInfo.commandBufferCount = 1;
    // Primary commands are directly sent to queues. Secondary ones are subcommands (used in multi-threaded scenarios)
    cmdAllocInfo.level = level;
    assert(vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmdBuf) == VK_SUCCESS);
    return cmdBuf;
}

VkFramebuffer VulkanContext::CreateFramebuffer(const VkDevice& device, const VkRenderPass& renderPass, const std::vector<VkImageView>& attachments, const VkExtent2D& extent) {
    Log::Debug("Creating Framebuffer...");

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    // can only be used with compatible (same number and type of attachments) render passes
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    VkFramebuffer framebuffer;
    assert(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer) == VK_SUCCESS);
    return framebuffer;
}

std::tuple<std::vector<VkFramebuffer>, VkImageView, AllocatedImage&> VulkanContext::CreateSwapChainFrameBuffers(const VkDevice& device, const VmaAllocator& allocator, const VkRenderPass& renderPass, const vr::SwapchainInfo& swapchainInfo) {
    Log::Debug("Creating Framebuffers for Swapchain...");
    // SwapChain creates images for color attachments automatically. 
    // If we want a depth attachment we need to create it manually.
    VkImageCreateInfo depthImageInfo = {};
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.pNext = nullptr;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.format = swapchainInfo.depthFormat;
    depthImageInfo.extent = { swapchainInfo.extent.width, swapchainInfo.extent.height, 1u }; // Depth extent is 3D
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VmaAllocationCreateInfo depthImageAllocationInfo = {};
    depthImageAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthImageAllocationInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    AllocatedImage depthImage = {};
    if (vmaCreateImage(allocator, &depthImageInfo, &depthImageAllocationInfo, &depthImage.image, &depthImage.allocation, nullptr) != VK_SUCCESS) {
        Log::Error("Cannot create/allocate depth image!");
        exit(EXIT_FAILURE);
    }

    VkImageView depthImageView;
    VkImageViewCreateInfo depthImageViewInfo = {};
    depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthImageViewInfo.pNext = nullptr;
    depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthImageViewInfo.image = depthImage.image;
    depthImageViewInfo.format = swapchainInfo.depthFormat;
    depthImageViewInfo.subresourceRange.baseMipLevel = 0;
    depthImageViewInfo.subresourceRange.levelCount = 1;
    depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
    depthImageViewInfo.subresourceRange.layerCount = 1;
    depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (vkCreateImageView(device, &depthImageViewInfo, nullptr, &depthImageView) != VK_SUCCESS) {
        Log::Error("Cannot create depth image view!");
        exit(EXIT_FAILURE);
    }

    const auto& presentImageViews = swapchainInfo.imageViews;
    std::vector<VkFramebuffer> presentFramebuffers(presentImageViews.size());
    for (size_t i = 0; i < presentImageViews.size(); i++) {
        // The swapchain framebuffers will share the same depth image
        std::vector<VkImageView> attachments = { presentImageViews[i], depthImageView };
        presentFramebuffers[i] = CreateFramebuffer(device, renderPass, attachments, swapchainInfo.extent);
    }
    return { presentFramebuffers, depthImageView, depthImage };
}

AllocatedBuffer VulkanContext::CreateAllocatedBuffer(const VmaAllocator& allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    AllocatedBuffer newBuffer = {};
    VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, nullptr);
    assert(result == VK_SUCCESS);

    return newBuffer;
}

VkDescriptorPool VulkanContext::CreateDescriptorPool(const VkDevice& device, const std::vector<VkDescriptorPoolSize>& sizes) {
    VkDescriptorPoolCreateInfo pool_info = {};
    uint32_t maxSets = 0;
    for (const auto& size : sizes) { maxSets += size.descriptorCount; }
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = maxSets;
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes = sizes.data();

    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);
    return descriptorPool;
}

void VulkanContext::OnResize(int width, int height) {
    Log::Debug("Framebuffer resized: ({}, {})", width, height);
    // TODO: resize logic will come here
}

void VulkanContext::drawFrame(const VkDevice& device, const VkSwapchainKHR& swapchain, const VkQueue& graphicsQueue, const VkRenderPass& renderPass, const std::vector<std::shared_ptr<IFrameData>>& frames, const std::vector<VkFramebuffer>& swapchainFramebuffers, const vr::SwapchainInfo& swapchainInfo, const std::vector<VkClearValue>& clearValues, std::function<void(const VkCommandBuffer&, uint32_t frameNo)> cmdFunc) {
    // Vulkan executes commands asynchroniously/independently. 
    // Need explicit dependency declaration for correct order of execution, i.e. synchronization
    // use fences to sync main app with command queue ops, use semaphors to sync operations within/across command queues
    static uint32_t frameNo = 0;
    uint32_t frameOverlap = static_cast<uint32_t>(frames.size());
    FrameSyncCmd frame = frames[frameNo % frameOverlap]->GetFrameSyncCmdData();

    // wait until the GPU has finished rendering the last frame. Timeout of 1 second
    assert(vkWaitForFences(device, 1, &frame.renderFence, true, 1000000000) == VK_SUCCESS); // 1sec = 1000000000
    assert(vkResetFences(device, 1, &frame.renderFence) == VK_SUCCESS);

    uint32_t swapchainImageIndex;
    // Request an imaage and wait until it's acquired (or timeout)
    VkResult result = vkAcquireNextImageKHR(device, swapchain, 1000000000, frame.presentSemaphore, nullptr, &swapchainImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // recreateSwapChain();
        assert(false);
        // TODO implement resize
    } else { 
        assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
    }

    // previous run of cmdBuf was finished, safely reset it to begin the recording again
    assert(vkResetCommandBuffer(frame.mainCommandBuffer, 0) == VK_SUCCESS);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // We'll use this buffer only once per frame
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    assert(vkBeginCommandBuffer(frame.mainCommandBuffer, &cmdBeginInfo) == VK_SUCCESS);

    VkRenderPassBeginInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = renderPass;
    // using render area we can render a smaller part of a bigger image
    rpInfo.renderArea.offset = { 0, 0 };
    rpInfo.renderArea.extent = swapchainInfo.extent;
    rpInfo.framebuffer = swapchainFramebuffers[swapchainImageIndex];
    rpInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    rpInfo.pClearValues = clearValues.data();
    // Will bind the Framebuffer, clear the Image, put the Image in the layout specified at RenderPass creation
    vkCmdBeginRenderPass(frame.mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Populate command buffer with the calls provided in the function
    cmdFunc(frame.mainCommandBuffer, frameNo % frameOverlap);

    // finishes rendering and transition image to "ready to be displayed" state that we specified
    vkCmdEndRenderPass(frame.mainCommandBuffer);
    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    assert(vkEndCommandBuffer(frame.mainCommandBuffer) == VK_SUCCESS);

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit.pWaitDstStageMask = &waitStage;
    submit.waitSemaphoreCount = 1;
    // Queue will wait until when the Swapchain is ready
    submit.pWaitSemaphores = &frame.presentSemaphore;
    submit.signalSemaphoreCount = 1;
    // When Queue processing is done on GPU it'll signal that rendering has finished
    submit.pSignalSemaphores = &frame.renderSemaphore;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &frame.mainCommandBuffer;
    // CPU submission will wait for GPU rendering to complete
    assert(vkQueueSubmit(graphicsQueue, 1, &submit, frame.renderFence) == VK_SUCCESS);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;
    // Present after GPU rendering is completed
    presentInfo.pWaitSemaphores = &frame.renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;
    assert(vkQueuePresentKHR(graphicsQueue, &presentInfo) == VK_SUCCESS);

    frameNo++;
}
