#include "VulkanRenderer.h"

#include "Core/Log.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>

#include <cassert>
#include <fstream>
#include <set>

VulkanRenderer::VulkanRenderer(VulkanContext& context) : vc(context) {}

VulkanRenderer::~VulkanRenderer() {
    //for (auto framebuffer : swapChainFramebuffers) {
    //    vkDestroyFramebuffer(vc.GetDevice(), framebuffer, nullptr);
    //}
    //vkDestroyPipeline(vc.GetDevice(), graphicsPipeline, nullptr);
    //vkDestroyPipelineLayout(vc.GetDevice(), pipelineLayout, nullptr);
}

VkCommandBuffer VulkanRenderer::CreateCommandBuffer(VkCommandBufferLevel level) {
    Log::Debug("Creating main, graphics Command Buffer...");
    VkCommandBuffer cmdBuf;
    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = vc.GetCommandPool();
    cmdAllocInfo.commandBufferCount = 1;
    // Primary commands are directly sent to queues. Secondary ones are subcommands (used in multi-threaded scenarios)
    cmdAllocInfo.level = level;
    assert(vkAllocateCommandBuffers(vc.GetDevice(), &cmdAllocInfo, &cmdBuf) == VK_SUCCESS);
    return cmdBuf;
}

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
    renderPassInfo.attachmentCount = attachments.size();
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
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
    // TODO: will come when loading vertex data
    //pipelineLayoutInfo.setLayoutCount = 1;
    //pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

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

void VulkanRenderer::UploadMesh(Mesh& mesh) {
    const auto& allocator = vc.GetAllocator();

    // Allocate vertex buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = mesh.vertices.size() * sizeof(Vertex); // total buffer size in bytes
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    Log::Debug("Allocating Vertex Buffer of size {} for mesh...", bufferInfo.size);
    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // Writeable by CPU, but also readable by GPU

    VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &mesh.vertexBuffer.buffer, &mesh.vertexBuffer.allocation, nullptr);
    assert(result == VK_SUCCESS);

    // Copy/Upload vertex data into vertex buffer
    Log::Debug("\tUploading mesh data...", bufferInfo.size);
    void* data;
    vmaMapMemory(allocator, mesh.vertexBuffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), bufferInfo.size);
    vmaUnmapMemory(allocator, mesh.vertexBuffer.allocation);
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
        }

        glm::mat4 MVP = renderView.projection * renderView.view * obj.transform;
        MeshPushConstants::PushConstant1 constants;
        constants.modelViewProjection = MVP;
        vkCmdPushConstants(cmd, obj.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant1), &constants);

        // Similarly, don't bind vertex buffer if we are repeating meshes
        if (obj.mesh != lastMesh) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &obj.mesh->vertexBuffer.buffer, &offset);
            lastMesh = obj.mesh;
        }

        vkCmdDraw(cmd, obj.mesh->vertices.size(), 1, 0, 0);
    }
}
