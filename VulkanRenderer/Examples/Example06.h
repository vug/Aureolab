#include "Example.h"
#include "Mesh.h"

#include <glm/gtx/transform.hpp>

VkPipelineLayout CreatePipelineLayout(
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

VkPipeline CreatePipeline(
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

// Empty but running example with no abstractions for command generation, frames-in-flight handling, pipeline creation etc.
class Ex06Plain : public Example {
public:
    Mesh mesh;
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

    Ex06Plain(VulkanContext& vc, VulkanRenderer& vr) : Example(vc, vr) {
        // Mesh Assets
        {
            mesh.LoadFromOBJ("assets/models/suzanne.obj");
            vr.UploadMesh(mesh);
            vr.meshes["monkey_flat"] = mesh;
            destroyer.Add(vr.meshes["monkey_flat"].vertexBuffer);
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
        float val = cos(time) * 0.5f + 0.5f;
        val *= 0.05f;
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = { val, 0.05f - val, 0.0f, 1.0f };
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
        // Example: drawing w/o mesh
        if (false) {
            vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.screenSquare);
            vkCmdDraw(mainCommandBuffer, 6, 1, 0, 0);
        }

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


        const auto& [width, height] = vc.GetSwapchainInfo().extent;
        VkViewport viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
        vkCmdSetViewport(mainCommandBuffer, 0, 1, &viewport);
        VkRect2D scissor = { {0, 0}, {width, height} };
        vkCmdSetScissor(mainCommandBuffer, 0, 1, &scissor);

        // Example simple mesh drawing
        // Bind Mesh
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offset);

        // Bind PushConstant for Model
        glm::mat4 worldFromObject = glm::mat4{ 1.0f };
        worldFromObject = glm::translate(worldFromObject, { 0, 0, 0.9 });
        worldFromObject = glm::scale(worldFromObject, { 0.5, 0.5, 0.5 });
        worldFromObject = glm::rotate(worldFromObject, time * 0.1f, glm::vec3(1, 0, 0));
        MeshPushConstants::PushConstant2 constants;
        // constants.data unused so far
        constants.transform = worldFromObject;
        vkCmdPushConstants(mainCommandBuffer, pipelines.normalLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant2), &constants);

        auto drawHalfWithWireframe = [&](VkPipeline pipeline, float ratio = 0.75) {
            int offsetX = int(width * ratio);
            scissor = { {0, 0}, {(uint32_t)offsetX, height} };
            vkCmdSetScissor(mainCommandBuffer, 0, 1, &scissor);
            vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdDraw(mainCommandBuffer, (uint32_t)mesh.vertices.size(), 1, 0, 0);

            scissor = { {offsetX, 0}, {width - offsetX, height} };
            vkCmdSetScissor(mainCommandBuffer, 0, 1, &scissor);
            vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.wireframe);
            vkCmdDraw(mainCommandBuffer, (uint32_t)mesh.vertices.size(), 1, 0, 0);
        };

        drawHalfWithWireframe(pipelines.normal);

        // Example textured mesh drawing
        vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offset);
        worldFromObject = glm::mat4{ 1.0f };
        worldFromObject = glm::translate(worldFromObject, { 1.5, 0, 0.0 });
        worldFromObject = glm::scale(worldFromObject, { 0.3, 0.3, 0.3 });
        worldFromObject = glm::rotate(worldFromObject, time * 2.0f, glm::vec3(0, 0, 1));
        constants.transform = worldFromObject;
        vkCmdPushConstants(mainCommandBuffer, pipelines.texturedLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant2), &constants);
        vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.texturedLayout, 1, 1, &descriptorSetTexture, 0, nullptr);

        drawHalfWithWireframe(pipelines.textured);


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