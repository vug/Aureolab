#include "Example.h"
#include "Mesh.h"

#include <glm/gtx/transform.hpp>

class FrameData06 : public IFrameData {
public:
    RenderView renderView;
};

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
    VkPipeline pipeline2, pipeline3;
    VkPipelineLayout pipelineLayout3;

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

            VkShaderModule vertShader3 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-push-const-vert.spv"));
            VkShaderModule fragShader3 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-push-const-frag.spv"));
            destroyer.Add(vertShader3);
            destroyer.Add(fragShader3);
            // VkShaderModule, VkShaderModule, VertexInputDescription, std::vector<VkPushConstantRange>, std::vector<VkDescriptorSetLayout>, VkRenderPass
            std::tie(pipeline3, pipelineLayout3) = vr.CreateSinglePassGraphicsPipeline(vertShader3, fragShader3, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), {}, vc.swapchainRenderPass);
            destroyer.Add(pipelineLayout3);
            destroyer.Add(pipeline3);
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
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = { val, 1.0f - val, 0.0f, 1.0f };
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
        // Example simple mesh drawing
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(mainCommandBuffer, 0, 1, &mesh.vertexBuffer.buffer, &offset);
        glm::vec3 camPos = { 0.f, 0.f, -2.f };
        glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
        glm::mat4 projection = glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f);
        projection[1][1] *= -1;
        glm::mat4 model = glm::mat4{ 1.0f };
        model = glm::translate(model, { 0, 0, 0.5 });
        model = glm::rotate(model, time, glm::vec3(0, 1, 0));
        glm::mat4 mvp = projection * view * model;
        MeshPushConstants::PushConstant1 constants;
        // constants.data empty
        constants.modelViewProjection = mvp;
        vkCmdPushConstants(mainCommandBuffer, pipelineLayout3, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant1), &constants);
        vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline3);
        vkCmdDraw(mainCommandBuffer, (uint32_t)mesh.vertices.size(), 1, 0, 0);

        // Example: no mesh drawing
        vkCmdBindPipeline(mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline2);
        vkCmdDraw(mainCommandBuffer, 6, 1, 0, 0);

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