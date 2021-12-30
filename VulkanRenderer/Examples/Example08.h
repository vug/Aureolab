#include "Example.h"
#include "Mesh.h"

#include <glm/gtx/transform.hpp>

#include <array>

static uint32_t GetMemoryType(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr)
{
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memTypeFound)
                {
                    *memTypeFound = true;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if (memTypeFound)
    {
        *memTypeFound = false;
        return 0;
    }
    else
    {
        throw std::runtime_error("Could not find a matching memory type");
    }
}

struct FrameBufferAttachment {
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};
struct OffscreenPass {
    int32_t width, height;
    VkFramebuffer frameBuffer;
    FrameBufferAttachment color, depth;
    VkRenderPass renderPass;
    VkSampler sampler;
    VkDescriptorImageInfo descriptor;
} offscreenPass;

static void prepareOffscreen(const VulkanContext& vc) {
    auto& device = vc.GetDevice();

    offscreenPass.width = 512;
    offscreenPass.height = 512;
    const VkFormat fbColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat fbDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT; // VK_FORMAT_D32_SFLOAT won't work because we have stencil info too
    // 1) PREPARE OFFSCREEN FRAMEBUFFER COLOR AND DEPTH ATTACHMENTS
    {
        // Color attachment
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = fbColorFormat;
        imageInfo.extent.width = offscreenPass.width;
        imageInfo.extent.height = offscreenPass.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        // We will sample directly from the color attachment
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        // manual allocation without vmaCreateImage
        VkMemoryAllocateInfo memAlloc = {};
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        assert(vkCreateImage(device, &imageInfo, nullptr, &offscreenPass.color.image) == VK_SUCCESS);
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, offscreenPass.color.image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = GetMemoryType(vc.physicalDevice.memoryProperties, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        assert(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPass.color.mem) == VK_SUCCESS);
        assert(vkBindImageMemory(device, offscreenPass.color.image, offscreenPass.color.mem, 0) == VK_SUCCESS);
        vc.destroyer->Add(offscreenPass.color.image);
        vc.destroyer->Add(offscreenPass.color.mem);

        VkImageViewCreateInfo colorImageView = {};
        colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorImageView.format = fbColorFormat;
        colorImageView.subresourceRange = {};
        colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageView.subresourceRange.baseMipLevel = 0;
        colorImageView.subresourceRange.levelCount = 1;
        colorImageView.subresourceRange.baseArrayLayer = 0;
        colorImageView.subresourceRange.layerCount = 1;
        colorImageView.image = offscreenPass.color.image;
        assert(vkCreateImageView(device, &colorImageView, nullptr, &offscreenPass.color.view) == VK_SUCCESS);
        vc.destroyer->Add(offscreenPass.color.view);

        // Create sampler to sample from the attachment in the fragment shader
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        assert(vkCreateSampler(device, &samplerInfo, nullptr, &offscreenPass.sampler) == VK_SUCCESS);
        vc.destroyer->Add(offscreenPass.sampler);

        // Depth stencil attachment
        // reuse color format CreateInfo
        imageInfo.format = fbDepthFormat;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        assert(vkCreateImage(device, &imageInfo, nullptr, &offscreenPass.depth.image) == VK_SUCCESS);
        vkGetImageMemoryRequirements(device, offscreenPass.depth.image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = GetMemoryType(vc.physicalDevice.memoryProperties, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        assert(vkAllocateMemory(device, &memAlloc, nullptr, &offscreenPass.depth.mem) == VK_SUCCESS);
        assert(vkBindImageMemory(device, offscreenPass.depth.image, offscreenPass.depth.mem, 0) == VK_SUCCESS);
        vc.destroyer->Add(offscreenPass.depth.image);
        vc.destroyer->Add(offscreenPass.depth.mem);

        VkImageViewCreateInfo depthStencilView = {};
        depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthStencilView.format = fbDepthFormat;
        depthStencilView.flags = 0;
        depthStencilView.subresourceRange = {};
        depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        depthStencilView.subresourceRange.baseMipLevel = 0;
        depthStencilView.subresourceRange.levelCount = 1;
        depthStencilView.subresourceRange.baseArrayLayer = 0;
        depthStencilView.subresourceRange.layerCount = 1;
        depthStencilView.image = offscreenPass.depth.image;
        assert(vkCreateImageView(device, &depthStencilView, nullptr, &offscreenPass.depth.view) == VK_SUCCESS);
        vc.destroyer->Add(offscreenPass.depth.view);
    }

    // 2) PREPARE OFFSCREEN RENDERPASS
	// Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering
    {
		std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
		// Color attachment
		attchmentDescriptions[0].format = fbColorFormat;
		attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		// Depth attachment
		attchmentDescriptions[1].format = fbDepthFormat;
		attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
        renderPassInfo.pAttachments = attchmentDescriptions.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        assert(vkCreateRenderPass(device, &renderPassInfo, nullptr, &offscreenPass.renderPass) == VK_SUCCESS);
        vc.destroyer->Add(offscreenPass.renderPass);
    }

    // 3) PREPARE OFFSCREEN FRAMEBUFFER
    {
        VkImageView attachments[2];
        attachments[0] = offscreenPass.color.view;
        attachments[1] = offscreenPass.depth.view;

        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = offscreenPass.renderPass;
        fbufCreateInfo.attachmentCount = 2;
        fbufCreateInfo.pAttachments = attachments;
        fbufCreateInfo.width = offscreenPass.width;
        fbufCreateInfo.height = offscreenPass.height;
        fbufCreateInfo.layers = 1;

        assert(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &offscreenPass.frameBuffer) == VK_SUCCESS);
        vc.destroyer->Add(offscreenPass.frameBuffer);
    }
}

static VkPipelineLayout CreatePipelineLayout(
    const VulkanContext& vc,
    const std::vector<VkPushConstantRange>& pushConstantRanges,
    const std::vector<VkDescriptorSetLayout>& descSetLayouts) {
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

    vc.destroyer->Add(pipelineLayout);
    return pipelineLayout;
}

static VkPipeline CreatePipeline(
    const VulkanContext& vc,
    VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
    const VertexInputDescription& vertDesc,
    VkPipelineLayout pipelineLayout,
    VkRenderPass renderPass,
    VkPolygonMode polygonMode,
    const std::vector<VkDynamicState>& dynamicStates
) {
    Log::Debug("Creating Graphics Pipeline...");

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
    //viewport.width = (float)vc.GetSwapchainInfo().extent.width / 3;
    viewport.width = (float)vc.GetSwapchainInfo().extent.width;
    viewport.height = (float)vc.GetSwapchainInfo().extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = vc.GetSwapchainInfo().extent;
    //scissor.extent.width /= 3;
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
    rasterizerInfo.polygonMode = polygonMode;
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

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

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
    pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
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
    vc.destroyer->Add(graphicsPipeline);

    return graphicsPipeline;
}

// Basic functional scene example with no abstractions for command generation, frames-in-flight handling, or pipeline creation etc.
class Ex08Offscreen : public Example {
public:
    VkPhysicalDeviceMemoryProperties memoryProperties;

    // Frame sync
    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    // CmdBufs
    VkCommandPool commandPool;
    VkCommandBuffer mainCommandBuffer;

    // Pipelines
    struct Pipelines {
        VkPipeline screenSquare, normal, textured, wireframe;
        VkPipelineLayout normalLayout, texturedLayout, wireframeLayout;
    };
    Pipelines pipelines;

    // Descriptors
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    VkDescriptorSet descriptorSetTexture;

    // RenderView
    RenderView renderView;

    Ex08Offscreen(VulkanContext& vc, VulkanRenderer& vr) : Example(vc, vr) {
        vkGetPhysicalDeviceMemoryProperties(vc.physicalDevice, &memoryProperties);
        prepareOffscreen(vc);

        // Mesh Assets
        {
            Mesh mesh;
            mesh.LoadFromOBJ("assets/models/suzanne.obj");
            vr.UploadMesh(mesh);
            vr.meshes["monkey_flat"] = mesh;
            destroyer.Add(vr.meshes["monkey_flat"].vertexBuffer);

            mesh.LoadFromOBJ("assets/models/cube.obj");
            vr.UploadMesh(mesh);
            vr.meshes["cube"] = mesh;
            destroyer.Add(vr.meshes["cube"].vertexBuffer);
        }

        // Frame Synchronization
        {
            VkFenceCreateInfo fenceCreateInfo = {};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            assert(vkCreateFence(vc.device, &fenceCreateInfo, nullptr, &renderFence) == VK_SUCCESS);

            //for the semaphores we don't need any flags
            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.flags = 0;
            assert(vkCreateSemaphore(vc.device, &semaphoreCreateInfo, nullptr, &presentSemaphore) == VK_SUCCESS);
            assert(vkCreateSemaphore(vc.device, &semaphoreCreateInfo, nullptr, &renderSemaphore) == VK_SUCCESS);

            destroyer.Add(presentSemaphore);
            destroyer.Add(renderSemaphore);
            destroyer.Add(renderFence);
        }

        // Command Buffers
        {
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = vc.GetQueueFamilyIndices().graphicsFamily.value();
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            if (vkCreateCommandPool(vc.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                Log::Debug("failed to create command pool!");
                exit(EXIT_FAILURE);
            }

            VkCommandBufferAllocateInfo cmdAllocInfo = {};
            cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdAllocInfo.pNext = nullptr;
            cmdAllocInfo.commandPool = commandPool;
            cmdAllocInfo.commandBufferCount = 1;
            cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            assert(vkAllocateCommandBuffers(vc.device, &cmdAllocInfo, &mainCommandBuffer) == VK_SUCCESS);

            destroyer.Add(commandPool);
        }

        // Descriptors
        VkDescriptorSetLayout singleTextureSetLayout;
        {
            descriptorPool = vc.CreateDescriptorPool(
                vc.GetDevice(), {
                    // Weirdly, works fine when commented out
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
                }
            );
            destroyer.Add(descriptorPool);

            VkDescriptorSetLayoutBinding texSetBind = {};
            texSetBind.binding = 0;
            texSetBind.descriptorCount = 1;
            texSetBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            texSetBind.pImmutableSamplers = nullptr;
            texSetBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo set3info = {};
            set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            set3info.bindingCount = 1;
            set3info.flags = 0;
            set3info.pBindings = &texSetBind;
            vkCreateDescriptorSetLayout(vc.GetDevice(), &set3info, nullptr, &singleTextureSetLayout);
            destroyer.Add(singleTextureSetLayout);
        }

        // RenderView
        {
            renderView.Init(vc.GetDevice(), vc.GetAllocator(), descriptorPool, vc.GetDestroyer());
            destroyer.Add(renderView.GetCameraBuffer());
        }

        descriptorSetLayouts.insert(descriptorSetLayouts.begin(), renderView.GetDescriptorSetLayouts().begin(), renderView.GetDescriptorSetLayouts().end());
        descriptorSetLayouts.push_back(singleTextureSetLayout);

        // Samplers
        VkSampler blockySampler;
        {
            VkSamplerCreateInfo samplerInfo = {};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            vkCreateSampler(vc.GetDevice(), &samplerInfo, nullptr, &blockySampler);
            destroyer.Add(blockySampler);
        }

        // Texture Assets
        {
            Texture texture;
            texture.LoadImageFromFile("assets/textures/texture.jpg");
            vr.UploadTexture(texture);
            vr.textures["sculpture"] = texture;
            destroyer.Add(vr.textures["sculpture"].imageView);
            destroyer.Add(vr.textures["sculpture"].newImage);

            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &singleTextureSetLayout;
            vkAllocateDescriptorSets(vc.GetDevice(), &allocInfo, &descriptorSetTexture);

            // Write to the descriptor set so that it points to given texture
            VkDescriptorImageInfo imageBufferInfo;
            imageBufferInfo.sampler = blockySampler;
            imageBufferInfo.imageView = vr.textures["sculpture"].imageView;
            imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet writeSetTexture = {};
            writeSetTexture.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeSetTexture.dstBinding = 0;
            writeSetTexture.dstSet = descriptorSetTexture;
            writeSetTexture.descriptorCount = 1;
            writeSetTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeSetTexture.pImageInfo = &imageBufferInfo;
            vkUpdateDescriptorSets(vc.GetDevice(), 1, &writeSetTexture, 0, nullptr);
        }

        // Pipeline
        {
            VkShaderModule vertShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-vert.spv"));
            VkShaderModule fragShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-frag.spv"));
            destroyer.Add(vertShader2);
            destroyer.Add(fragShader2);
            VkPipelineLayout pipelineLayout2;
            std::tie(pipelines.screenSquare, pipelineLayout2) = vr.CreateSinglePassGraphicsPipeline(vertShader2, fragShader2, {}, {}, {}, vc.swapchainRenderPass);
            destroyer.Add(pipelineLayout2);
            destroyer.Add(pipelines.screenSquare);

            VkShaderModule vertShader3 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-04-desc-set-vert.spv"));
            VkShaderModule fragShader3 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-normal-frag.spv"));
            pipelines.normalLayout = CreatePipelineLayout(vc, { MeshPushConstants::GetPushConstantRange<MeshPushConstants::PushConstant2>() }, descriptorSetLayouts);
            pipelines.normal = CreatePipeline(vc, vertShader3, fragShader3, Vertex::GetVertexDescription(), pipelines.normalLayout, vc.swapchainRenderPass,
                VK_POLYGON_MODE_FILL, { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
            destroyer.Add(std::vector{ vertShader3, fragShader3 });

            VkShaderModule vertShader4 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-05-textured-vert.spv"));
            VkShaderModule fragShader4 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-05-textured-frag.spv"));
            pipelines.texturedLayout = CreatePipelineLayout(vc, { MeshPushConstants::GetPushConstantRange<MeshPushConstants::PushConstant2>() }, descriptorSetLayouts);
            pipelines.textured = CreatePipeline(vc, vertShader4, fragShader4, Vertex::GetVertexDescription(), pipelines.texturedLayout, vc.swapchainRenderPass,
                VK_POLYGON_MODE_FILL, { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR });
            destroyer.Add(std::vector{ vertShader4, fragShader4 });

            VkShaderModule vertShader5 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/default-vert.spv"));
            VkShaderModule fragShader5 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-solid-color-frag.spv"));
            pipelines.wireframeLayout = CreatePipelineLayout(vc, { MeshPushConstants::GetPushConstantRange<MeshPushConstants::PushConstant2>() }, descriptorSetLayouts);
            pipelines.wireframe = CreatePipeline(vc, vertShader5, fragShader5, Vertex::GetVertexDescription(), pipelines.wireframeLayout, vc.swapchainRenderPass,
                VK_POLYGON_MODE_LINE, { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }); // VK_DYNAMIC_STATE_LINE_WIDTH,
            destroyer.Add(std::vector{ vertShader5, fragShader5 });
        }
    }

    void OnRender(float time, float delta) {
        // --- BEGIN FRAME
        assert(vkWaitForFences(vc.device, 1, &renderFence, true, 1000000000) == VK_SUCCESS); // 1sec = 1000000000
        assert(vkResetFences(vc.device, 1, &renderFence) == VK_SUCCESS);

        uint32_t swapchainImageIndex;
        VkResult result = vkAcquireNextImageKHR(vc.device, vc.swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex);
        // TODO: handle resize. if (result == VK_ERROR_OUT_OF_DATE_KHR)
        assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
        // --- BEGIN FRAME


        // --- FILL COMMAND BUFFER
        assert(vkResetCommandBuffer(mainCommandBuffer, 0) == VK_SUCCESS);
        VkCommandBufferBeginInfo cmdBeginInfo = {};
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        assert(vkBeginCommandBuffer(mainCommandBuffer, &cmdBeginInfo) == VK_SUCCESS);

        // Commands for early passes

        // Example: Update Clear Color
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil.depth = 1.0f;

        VkRenderPassBeginInfo rpInfo = {};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpInfo.renderPass = vc.swapchainRenderPass;
        rpInfo.renderArea.offset = { 0, 0 };
        rpInfo.renderArea.extent = vc.swapchain.swapchainInfo.extent;
        rpInfo.framebuffer = vc.swapchainFramebuffers[swapchainImageIndex];
        rpInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        rpInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Commands for presentation pass
        // Cursor coordinates if needed
        const auto& [mx, my] = vc.win.GetMouseCursorPosition();

        // Bind RenderView (Camera) UBO for ViewProjection
        {
            float r = 2.0f;
            glm::vec3 camPos = { r * sin(time), 0.f, r * cos(time) };
            glm::mat4 viewFromWorld = glm::lookAt(camPos, { 0,0,0 }, { 0,1,0 });
            glm::mat4 projectionFromView = glm::perspective(glm::radians(70.f), (float)vc.GetSwapchainInfo().extent.width / vc.GetSwapchainInfo().extent.height, 0.1f, 200.0f);
            renderView.camera = { viewFromWorld, projectionFromView };
            renderView.camera.projection[1][1] *= -1;
        }
        vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.normalLayout, 0, 1, &renderView.GetDescriptorSet(), 0, nullptr);

        const AllocatedBuffer& camBuf = renderView.GetCameraBuffer();
        void* data;
        vmaMapMemory(vc.GetAllocator(), camBuf.allocation, &data);
        memcpy(data, &renderView.camera, sizeof(RenderView::Camera));
        vmaUnmapMemory(vc.GetAllocator(), camBuf.allocation);

        // Needed only when pipeline has dynamic states for viewport and scissors
        const auto& [width, height] = vc.GetSwapchainInfo().extent;
        VkViewport viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
        vkCmdSetViewport(mainCommandBuffer, 0, 1, &viewport);
        VkRect2D scissor = { {0, 0}, {width, height} };
        vkCmdSetScissor(mainCommandBuffer, 0, 1, &scissor);

        // Example simple mesh drawing
        // Bind Mesh
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, &vr.meshes["monkey_flat"].vertexBuffer.buffer, &offset);

        // Bind PushConstant for Model
        glm::mat4 worldFromObject = glm::mat4{ 1.0f };
        worldFromObject = glm::translate(worldFromObject, { 0, 0, 0.9 });
        worldFromObject = glm::scale(worldFromObject, { 0.5, 0.5, 0.5 });
        worldFromObject = glm::rotate(worldFromObject, time * 0.1f, glm::vec3(1, 0, 0));
        MeshPushConstants::PushConstant2 constants;
        // constants.data unused so far
        constants.transform = worldFromObject;
        vkCmdPushConstants(mainCommandBuffer, pipelines.normalLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant2), &constants);
        vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.normal);
        vkCmdDraw(mainCommandBuffer, (uint32_t)vr.meshes["monkey_flat"].vertices.size(), 1, 0, 0);

        // Example textured mesh drawing
        vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, &vr.meshes["cube"].vertexBuffer.buffer, &offset);
        worldFromObject = glm::mat4{ 1.0f };
        worldFromObject = glm::translate(worldFromObject, { 1.5, 0, 0.0 });
        worldFromObject = glm::scale(worldFromObject, { 0.3, 0.3, 0.3 });
        worldFromObject = glm::rotate(worldFromObject, time * 2.0f, glm::vec3(0, 0, 1));
        constants.transform = worldFromObject;
        vkCmdPushConstants(mainCommandBuffer, pipelines.texturedLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant2), &constants);
        vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.texturedLayout, 1, 1, &descriptorSetTexture, 0, nullptr);
        vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.textured);
        vkCmdDraw(mainCommandBuffer, (uint32_t)vr.meshes["cube"].vertices.size(), 1, 0, 0);

        vkCmdEndRenderPass(mainCommandBuffer);
        assert(vkEndCommandBuffer(mainCommandBuffer) == VK_SUCCESS);
        // --- FILL COMMAND BUFFER


        // --- END FRAME
        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit.pWaitDstStageMask = &waitStage;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &presentSemaphore;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &renderSemaphore;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &mainCommandBuffer;
        assert(vkQueueSubmit(vc.GetGraphicsQueue(), 1, &submit, renderFence) == VK_SUCCESS);

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = vc.swapchain;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &renderSemaphore;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &swapchainImageIndex;
        assert(vkQueuePresentKHR(vc.GetGraphicsQueue(), &presentInfo) == VK_SUCCESS);
        // --- END FRAME
    }
};