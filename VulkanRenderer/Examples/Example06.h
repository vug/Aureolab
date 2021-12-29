#include "Example.h"
#include "Mesh.h"

#include <glm/gtx/transform.hpp>

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
    VkPipeline pipeline2, pipeline3, pipeline4;
    VkPipelineLayout pipelineLayout3, pipelineLayout4;

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
            std::tie(pipeline2, pipelineLayout2) = vr.CreateSinglePassGraphicsPipeline(vertShader2, fragShader2, {}, {}, {}, vc.swapchainRenderPass);
            destroyer.Add(pipelineLayout2);
            destroyer.Add(pipeline2);

            VkShaderModule vertShader3 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-04-desc-set-vert.spv"));
            VkShaderModule fragShader3 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/visualize-normal-frag.spv"));
            destroyer.Add(vertShader3);
            destroyer.Add(fragShader3);
            std::tie(pipeline3, pipelineLayout3) = vr.CreateSinglePassGraphicsPipeline(vertShader3, fragShader3, Vertex::GetVertexDescription(), { MeshPushConstants::GetPushConstantRange<MeshPushConstants::PushConstant2>() }, descriptorSetLayouts, vc.swapchainRenderPass);
            destroyer.Add(pipelineLayout3);
            destroyer.Add(pipeline3);

            VkShaderModule vertShader4 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-05-textured-vert.spv"));
            VkShaderModule fragShader4 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-05-textured-frag.spv"));
            std::tie(pipeline4, pipelineLayout4) = vr.CreateSinglePassGraphicsPipeline(vertShader4, fragShader4, Vertex::GetVertexDescription(), { MeshPushConstants::GetPushConstantRange<MeshPushConstants::PushConstant2>() }, descriptorSetLayouts, vc.swapchainRenderPass);
            destroyer.Add(std::vector{ vertShader4, fragShader4 });
            destroyer.Add(pipelineLayout4);
            destroyer.Add(pipeline4);
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
            vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline2);
            vkCmdDraw(mainCommandBuffer, 6, 1, 0, 0);
        }

        // Bind RenderView (Camera) UBO for ViewProjection
        {
            float r = 2.0f;
            glm::vec3 camPos = { r * sin(time), 0.f, r * cos(time) };
            glm::mat4 viewFromWorld = glm::lookAt(camPos, { 0,0,0 }, { 0,1,0 });
            glm::mat4 projectionFromView = glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f);
            renderView.camera = { viewFromWorld, projectionFromView };
            renderView.camera.projection[1][1] *= -1;
        }
        vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout3, 0, 1, &renderView.GetDescriptorSet(), 0, nullptr);

        const AllocatedBuffer& camBuf = renderView.GetCameraBuffer();
        void* data;
        vmaMapMemory(vc.GetAllocator(), camBuf.allocation, &data);
        memcpy(data, &renderView.camera, sizeof(RenderView::Camera));
        vmaUnmapMemory(vc.GetAllocator(), camBuf.allocation);

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
        vkCmdPushConstants(mainCommandBuffer, pipelineLayout3, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant2), &constants);

        vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline3);
        vkCmdDraw(mainCommandBuffer, (uint32_t)mesh.vertices.size(), 1, 0, 0);

        // Example textured mesh drawing
        vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offset);
        worldFromObject = glm::mat4{ 1.0f };
        worldFromObject = glm::translate(worldFromObject, { 1.5, 0, 0.0 });
        worldFromObject = glm::scale(worldFromObject, { 0.3, 0.3, 0.3 });
        worldFromObject = glm::rotate(worldFromObject, time * 2.0f, glm::vec3(0, 0, 1));
        constants.transform = worldFromObject;
        vkCmdPushConstants(mainCommandBuffer, pipelineLayout4, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant2), &constants);

        vkCmdBindDescriptorSets(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout4, 1, 1, &descriptorSetTexture, 0, nullptr);

        vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline4);
        vkCmdDraw(mainCommandBuffer, (uint32_t)mesh.vertices.size(), 1, 0, 0);


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