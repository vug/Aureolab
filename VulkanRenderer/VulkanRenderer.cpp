#include "VulkanRenderer.h"

#include "Core/Log.h"

#include <vulkan/vulkan.h>

#include <fstream>
#include <set>

VulkanRenderer::VulkanRenderer(VulkanContext& context) : vc(context) {}

VulkanRenderer::~VulkanRenderer() {
    //for (auto framebuffer : swapChainFramebuffers) {
    //    vkDestroyFramebuffer(vc.GetDevice(), framebuffer, nullptr);
    //}
    //vkDestroyPipeline(vc.GetDevice(), graphicsPipeline, nullptr);
    //vkDestroyPipelineLayout(vc.GetDevice(), pipelineLayout, nullptr);
    //vkDestroyRenderPass(vc.GetDevice(), renderPass, nullptr);

}

void VulkanRenderer::OnResize(int width, int height) {
    Log::Debug("Framebuffer resized: ({}, {})", width, height);
    // TODO: resize logic will come here
}

void VulkanRenderer::CreateExampleGraphicsPipeline(const std::string& vertFilename, const std::string& fragFilename) {
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

    if (vkCreateRenderPass(vc.GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        Log::Critical("failed to create render pass!");
        exit(EXIT_FAILURE);
    }


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
    pipelineInfo.pDepthStencilState = nullptr; // 
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

    if (vkCreateGraphicsPipelines(vc.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        Log::Critical("failed to create graphics pipeline!");
        exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(vc.GetDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(vc.GetDevice(), vertShaderModule, nullptr);

    Log::Debug("\tCreating Framebuffers for Swapchain Images...");
    swapChainFramebuffers.resize(vc.GetSwapchainInfo().imageViews.size());
    for (size_t i = 0; i < vc.GetSwapchainInfo().imageViews.size(); i++) {
        VkImageView attachments[] = {
            vc.GetSwapchainInfo().imageViews[i],
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        // can only be used with compatible (same number and type of attachments) render passes
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vc.GetSwapchainInfo().extent.width;
        framebufferInfo.height = vc.GetSwapchainInfo().extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vc.GetDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            Log::Debug("failed to create framebuffer!");
            exit(EXIT_FAILURE);
        }
    }
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