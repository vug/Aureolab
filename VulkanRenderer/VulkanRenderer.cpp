#include "VulkanRenderer.h"

#include "Core/Log.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>

#include <cassert>
#include <fstream>
#include <set>

// RenderView
void RenderView::Init(const VkDevice& device, const VmaAllocator& allocator, const VkDescriptorPool& pool, VulkanDestroyer& destroyer) {
    cameraBuffer = VulkanContext::CreateAllocatedBuffer(allocator, sizeof(RenderView::Camera), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    descriptorSetLayouts = CreateDescriptorSetLayouts(device, destroyer);
    descriptorSet = AllocateAndUpdateDescriptorSet(device, pool, descriptorSetLayouts, cameraBuffer);
}

std::vector<VkDescriptorSetLayout> RenderView::CreateDescriptorSetLayouts(const VkDevice& device, VulkanDestroyer& destroyer) {
    std::vector<VkDescriptorSetLayout> layouts;

    VkDescriptorSetLayoutBinding camBufferBinding = {};
    camBufferBinding.binding = 0;
    camBufferBinding.descriptorCount = 1;
    camBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    camBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // used by the vertex shader
    VkDescriptorSetLayoutCreateInfo setInfo = {};
    setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setInfo.bindingCount = 1;
    setInfo.flags = 0;
    setInfo.pBindings = &camBufferBinding;
    VkDescriptorSetLayout globalSetLayout;
    vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &globalSetLayout);
    layouts.push_back(globalSetLayout);
    destroyer.Add(globalSetLayout);

    return layouts;
}

VkDescriptorSet RenderView::AllocateAndUpdateDescriptorSet(const VkDevice& device, const VkDescriptorPool& pool, const std::vector<VkDescriptorSetLayout>& layouts, const AllocatedBuffer& cameraBuffer) {
    VkDescriptorSet set;

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();
    vkAllocateDescriptorSets(device, &allocInfo, &set);

    VkDescriptorBufferInfo bufInfo;
    bufInfo.buffer = cameraBuffer.buffer;
    bufInfo.offset = 0;
    bufInfo.range = sizeof(RenderView::Camera);
    VkWriteDescriptorSet setWrite = {};
    setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    setWrite.dstBinding = 0;
    setWrite.dstSet = set;
    setWrite.descriptorCount = 1;
    setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setWrite.pBufferInfo = &bufInfo;
    vkUpdateDescriptorSets(device, 1, &setWrite, 0, nullptr);

    return set;
}

// VulkanRenderer

VulkanRenderer::VulkanRenderer(VulkanContext& context) :
    vc(context),
    imCmdSubmitter(vc.GetDevice(), vc.GetGraphicsQueue(), vc.GetQueueFamilyIndices().graphicsFamily.value(), vc.GetDestroyer()) {}

VulkanRenderer::~VulkanRenderer() {}

// Single-Pass "Presenting" RenderPass
VkRenderPass VulkanRenderer::CreateRenderPass() {
    Log::Debug("Creating Render Pass...");
    /*
     A Renderpass is a collection of N attachments, M subpassesand their L dependencies.
     Describes how attachments are used over the course of subpasses.
     Each subpass can use N_m <= N subset of attachments as their input, color, depth, resolve etc. attachment
     Dependencies are memory-deps between src and dst subpasses. Self-dependencies are possible.

     Later, Graphics Pipelines and Framebuffers will be created with this Render pass, which ensures compatibility.
    */
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vc.GetSwapchainInfo().surfaceFormat.format;
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
    depthAttachment.format = vc.GetSwapchainInfo().depthFormat;
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
    assert(vkCreateRenderPass(vc.GetDevice(), &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS);
    return renderPass;
}

std::tuple<VkPipeline, VkPipelineLayout> VulkanRenderer::CreateSinglePassGraphicsPipeline(
    VkShaderModule& vertShaderModule, VkShaderModule& fragShaderModule, 
    const VertexInputDescription& vertDesc, 
    const std::vector<VkPushConstantRange>& pushConstantRanges,
    const std::vector<VkDescriptorSetLayout>& descSetLayouts,
    VkRenderPass& renderPass
) {
    Log::Debug("Creating Graphics Pipeline...");
    // optional parameters: shaders, Vertex class with binding and attribute descriptions,
    // VkPrimitiveTopology topology, VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace
    // VkSampleCountFlagBits rasterizationSamples, VkPipelineColorBlendAttachmentState alpha blend settings

    Log::Debug("\tCreating Shader Stage Info...");
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
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertDesc.bindings.size());
    vertexInputInfo.pVertexBindingDescriptions = vertDesc.bindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertDesc.attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertDesc.attributes.data();

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
    viewport.width = (float)vc.GetSwapchainInfo().extent.width;
    viewport.height = (float)vc.GetSwapchainInfo().extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = vc.GetSwapchainInfo().extent;
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
    rasterizerInfo.cullMode = VK_CULL_MODE_NONE;
    //rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
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
    depthStencilInfo.pNext = nullptr;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.minDepthBounds = 0.0f; // Optional
    depthStencilInfo.maxDepthBounds = 1.0f; // Optional
    depthStencilInfo.stencilTestEnable = VK_FALSE;

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
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();

    if (vkCreatePipelineLayout(vc.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        Log::Critical("failed to create pipeline layout!");
        exit(EXIT_FAILURE);
    }

    Log::Debug("\tCreating Pipeline...");
    // Made of 1) Shader stages, 2) Fixed-function states,
    // 3) Pipeline Layout (uniform/push values used by shaders, updated at draw time), 4) Renderpass
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // 1) Shaders
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    // 2) Fixed-function states
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisamplingInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pColorBlendState = &colorBlendingInfo;
    pipelineInfo.pDynamicState = nullptr;
    // 3) Pipeline Layout
    pipelineInfo.layout = pipelineLayout;
    // 4) Renderpass
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    // could have created (derive) this pipeline based on another pipeline
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(vc.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        Log::Critical("failed to create graphics pipeline!");
        exit(EXIT_FAILURE);
    }

    return { graphicsPipeline, pipelineLayout };
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
    if (vkCreateShaderModule(vc.GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void VulkanRenderer::UploadMeshCpuToGpu(Mesh& mesh) {
    const auto& allocator = vc.GetAllocator();

    size_t size = mesh.vertices.size() * sizeof(Vertex);
    mesh.vertexBuffer = vc.CreateAllocatedBuffer(allocator, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    Log::Debug("\tUploading mesh data (copy mesh's vertex data into allocated CPU_TO_GPU vertex buffer) of size {}...", size);
    void* data;
    vmaMapMemory(allocator, mesh.vertexBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), size);
    vmaUnmapMemory(allocator, mesh.vertexBuffer.allocation);
}

void VulkanRenderer::UploadMesh(Mesh& mesh) {
    const auto& allocator = vc.GetAllocator();

    size_t size = mesh.vertices.size() * sizeof(Vertex);
    AllocatedBuffer stagingBuffer = vc.CreateAllocatedBuffer(allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    Log::Debug("\tUploading mesh data (copy mesh's vertex data into allocated CPU_ONLY staging buffer) of size {}...", size);
    void* data;
    vmaMapMemory(allocator, stagingBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), size);
    vmaUnmapMemory(allocator, stagingBuffer.allocation);

    mesh.vertexBuffer = vc.CreateAllocatedBuffer(allocator, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    Log::Debug("\tUploading mesh data (submit a command to transfer staging buffer to GPU_ONLY VRAM) of size {}...", size);
    imCmdSubmitter.Submit([=](VkCommandBuffer cmd) {
        VkBufferCopy copy;
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size = size;
        vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh.vertexBuffer.buffer, 1, &copy);
    });

    // vc.GetDestroyer().Add(mesh.vertexBuffer); // for some reason adding vertexBuffer to deletion queue here does not free it in time at VulkanContext destructor DestroyAll call
    vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

void VulkanRenderer::UploadTexture(Texture& tex) {
    Log::Debug("Uploading pixels into an Image/Texture...");
    const auto& allocator = vc.GetAllocator();
    auto& destroyer = vc.GetDestroyer();

    Log::Debug("\tCopying pixel data into a staging buffer as TRANSFER_SRC and CPU_ONLY...");
    void* pixel_ptr = tex.pixels;
    // assuming all loaded images are converted into 32-bit RBGA format
    VkDeviceSize imageSize = 4ULL * tex.width * tex.height;
    // the format R8G8B8A8 matches exactly with the pixels loaded from stb_image lib
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

    AllocatedBuffer stagingBuffer = vc.CreateAllocatedBuffer(allocator, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    void* data;
    vmaMapMemory(allocator, stagingBuffer.allocation, &data);
    memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
    vmaUnmapMemory(allocator, stagingBuffer.allocation);
    
    stbi_image_free(tex.pixels); // data in CPU mem no longer needed

    Log::Debug("\tCreating and Allocating an Image as TRANSFER_DST and GPU_ONLY...");
    VkExtent3D imageExtent;
    imageExtent.width = static_cast<uint32_t>(tex.width);
    imageExtent.height = static_cast<uint32_t>(tex.height);
    imageExtent.depth = 1;
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = imageExtent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vmaCreateImage(allocator, &imageInfo, &allocInfo, &tex.newImage.image, &tex.newImage.allocation, nullptr);

    Log::Debug("\tChanging the layout of the image from UNDEFINED to TRANSFER_DST_OPTIMAL via a pipeline barrier...");
    // before copying staging buffer into it do it by submitting pipeline barrier command
    imCmdSubmitter.Submit([&](VkCommandBuffer cmdBuf) {
        // simplest possible image, only transform the single layer/level, no mip-map...
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        VkImageMemoryBarrier imageBarrier_toTransfer = {};
        imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // from
        imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // to
        imageBarrier_toTransfer.image = tex.newImage.image;
        imageBarrier_toTransfer.subresourceRange = range;
        imageBarrier_toTransfer.srcAccessMask = 0;
        imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        // Barrier the image into the transfer-receive layout
        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

        Log::Debug("\tCopying staging buffer into image...");
        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageExtent = imageExtent;
        vkCmdCopyBufferToImage(cmdBuf, stagingBuffer.buffer, tex.newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        Log::Debug("\tChanging the layout of the image from TRANSFER_DST_OPTIMAL into a shader readable format SHADER_READ_ONLY_OPTIMAL via a pipeline barrier...");
        VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
        imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // from
        imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // to
        imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
    });

    vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

glm::mat4 VulkanRenderer::MakeTransform(const glm::vec3& translate, const glm::vec3& axis, float angle, const glm::vec3& scale) {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, translate);
    transform = glm::scale(transform, scale);
    transform = glm::rotate(transform, angle, axis);
    return transform;
}

void VulkanRenderer::DrawObjects(VkCommandBuffer cmd, RenderView& renderView, std::vector<RenderObject> objects) {
    Mesh* lastMesh = nullptr;
    Material* lastMaterial = nullptr;
    for (const auto& obj : objects) {
        // Only bind pipeline if it changes while looping over objects
        if (obj.material != lastMaterial) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, obj.material->pipeline);
            lastMaterial = obj.material;
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, obj.material->pipelineLayout, 0, 1, &renderView.GetDescriptorSet(), 0, nullptr);
        }

        MeshPushConstants::PushConstant1 constants;
        constants.modelViewProjection = obj.transform;
        vkCmdPushConstants(cmd, obj.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant1), &constants);

        // Similarly, don't bind vertex buffer if we are repeating meshes
        if (obj.mesh != lastMesh) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &obj.mesh->vertexBuffer.buffer, &offset);
            lastMesh = obj.mesh;
        }

        vkCmdDraw(cmd, static_cast<uint32_t>(obj.mesh->vertices.size()), 1, 0, 0);
    }
}
