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

VulkanContext::VulkanContext(VulkanWindow& win)
    : instance(vr::InstanceBuilder()),
    // TODO: should depend on instance->builder.params.headless
    surface(vr::SurfaceBuilder(instance, win)),
    physicalDevice(vr::PhysicalDeviceBuilder(instance, surface)),
    device(vr::DeviceBuilder(physicalDevice)),
    allocator(vr::AllocatorBuilder(device)),
    destroyer(std::make_unique<VulkanDestroyer>(device, allocator)),
    swapchain(vr::SwapchainBuilder(device)),
    swapchainRenderPass(initSwapchainRenderPass()),
    swapchainFramebuffers(initSwapchainFramebuffers()) {
    /* Potential configuration options:
    - Device features such as samplerAnisotropy
    - Number of images in SwapChain
    */
    Log::Debug("Constructing Vulkan Context...");

    // Window binding. Will make the window to call OnResize when Window's framebuffer resized.
    win.SetUserPointer(this);

    // TODO: better to use std::optional and an initDebugMessenger private method for initialization
    if (instance.builder.params.validation)
        debugMessenger = std::make_unique<vr::DebugMessenger>(vr::DebugMessengerBuilder(instance));
}

VulkanContext::~VulkanContext() {
    Log::Debug("Destructing Vulkan Context...");
    destroyer->DestroyAll();
    vkDestroyRenderPass(device, swapchainRenderPass, nullptr);
}

VkRenderPass VulkanContext::initSwapchainRenderPass() {
    /*
     A Renderpass is a collection of N attachments, M subpassesand their L dependencies.
     Describes how attachments are used over the course of subpasses.
     Each subpass can use N_m <= N subset of attachments as their input, color, depth, resolve etc. attachment
     Dependencies are memory-deps between src and dst subpasses. Self-dependencies are possible.

     Later, Graphics Pipelines and Framebuffers will be created with this Render pass, which ensures compatibility.
    */
    Log::Debug("Creating Render Pass for Swapchain...");
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain.swapchainInfo.surfaceFormat.format;
    // should this be the same as pipeline rasterizationSamples?
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // at the beginning of subpass clear when loaded: _LOAD would preserved what was previously there. Other: _DONT_CARE
    // for depth/stencil attachment only apply to depth
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // store it at the end of subpass: or _DONT_CARE
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
    // index in parent Renderpass' VkAttachmentDescription* pAttachments
    colorAttachmentRef.attachment = 0;
    // Vulkan will transition the attachment into this layout when subpass will start
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.flags = 0;
    depthAttachment.format = swapchain.swapchainInfo.depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Need at least one sub-pass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // not _COMPUTE
    subpass.colorAttachmentCount = 1;
    // index in this "array" is directly referenced in fragment shader, say `layout(location = 0) out vec4 outColor`
    subpass.pColorAttachments = &colorAttachmentRef;
    // Life of an image: UNDEFINED -> RenderPass Begins -> Subpass 0 begins (Transition to Attachment Optimal) -> Subpass 0 renders -> Subpass 0 ends -> Renderpass Ends (Transitions to Present Source)
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // specify memory and execution dependencies between subpasses
    // We want our only single subpass to happen after swap chain image is acquired
    // Option 1: set waitStages for the imageAvailableSemaphore to VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT <- render pass does not start until image is available
    // Option 2: (this one) make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage
    //VkSubpassDependency dependency{};
    //dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // special value reference: "subpass before renderpass"
    //dependency.dstSubpass = 0; // first and only subpass
    //dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependency.srcAccessMask = 0;
    //dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Renderpass is created by providing attachments, subpasses that use them, and the dependency relationship between subpasses
    std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    //renderPassInfo.dependencyCount = 1;
    //renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass;
    VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS) {
        Log::Debug("Failed to create RenderPass for Swapchain!");
        exit(EXIT_FAILURE);
    }
    return renderPass;
}

std::vector<VkFramebuffer> VulkanContext::initSwapchainFramebuffers() {
    Log::Debug("Creating Framebuffers for Swapchain...");
    // SwapChain creates images for color attachments automatically. 
    // If we want a depth attachment we need to create it manually.
    auto& swapchainInfo = swapchain.swapchainInfo;
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
        Log::Debug("Creating Swapchain Framebuffer [{}]...", i);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // can only be used with compatible (same number and type of attachments) render passes
        framebufferInfo.renderPass = swapchainRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainInfo.extent.width;
        framebufferInfo.height = swapchainInfo.extent.height;
        framebufferInfo.layers = 1;

        assert(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &presentFramebuffers[i]) == VK_SUCCESS);
    }
    // TODO: Destroy these in Framebuffer abstraction destructor
    destroyer->Add(presentFramebuffers);
    destroyer->Add(depthImageView);
    destroyer->Add(depthImage);
    return presentFramebuffers;
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
