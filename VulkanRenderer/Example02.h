#pragma once

#include "Example.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>

#include <chrono>

class Ex02VertexBufferInput : public Example {
public:
    Ex02VertexBufferInput(VulkanContext& vc, VulkanRenderer& vr) :
        Example(vc, vr) {
        triangleMesh.MakeTriangle();
        vr.UploadMesh(triangleMesh);
        destroyer.Add(triangleMesh.vertexBuffer);
        quadMesh.MakeQuad();
        vr.UploadMesh(quadMesh);
        destroyer.Add(quadMesh.vertexBuffer);
        monkeyMesh.LoadFromOBJ("assets/models/suzanne.obj");
        vr.UploadMesh(monkeyMesh);
        destroyer.Add(monkeyMesh.vertexBuffer);

        renderPass = vr.CreateRenderPass();
        destroyer.Add(renderPass);
        AllocatedImage depthImage;
        VkImageView depthImageView;
        std::tie(presentFramebuffers, depthImageView, depthImage) = vc.CreateSwapChainFrameBuffers(vc.GetDevice(), vc.GetAllocator(), renderPass, vc.GetSwapchainInfo());
        destroyer.Add(presentFramebuffers);
        destroyer.Add(depthImageView);
        destroyer.Add(depthImage);

        frameSyncCmd = vc.CreateFrameSyncCmd(vc.GetDevice(), vc.GetQueueFamilyIndices().graphicsFamily.value());
        destroyer.Add(frameSyncCmd.commandPool);
        destroyer.Add(frameSyncCmd.renderFence);
        destroyer.Add(std::vector{ frameSyncCmd.presentSemaphore, frameSyncCmd.renderSemaphore });

        VkShaderModule vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-push-const-vert.spv"));
        VkShaderModule fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-push-const-frag.spv"));
        destroyer.Add(vertShader);
        destroyer.Add(fragShader);
        std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), renderPass);
        destroyer.Add(pipelineLayout);
        destroyer.Add(pipeline);
    }

    void OnRender() {
        static auto t0 = std::chrono::system_clock::now();
        std::chrono::duration<float> time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t0);

        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil.depth = 1.0f;
        vc.drawFrameBlocked(vc.GetDevice(), vc.GetSwapchain(), vc.GetGraphicsQueue(), renderPass, frameSyncCmd, presentFramebuffers, vc.GetSwapchainInfo(), clearValues, [&](const VkCommandBuffer& cmd) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            Mesh mesh = int(time.count() / 2.0f) % 2 == 0 ? monkeyMesh : quadMesh;

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);

            glm::vec3 camPos = { 0.f, 0.f, -2.f };
            glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
            glm::mat4 projection = glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f);
            projection[1][1] *= -1;
            glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, time.count(), glm::vec3(0, 1, 0));
            glm::mat4 mvp = projection * view * model;
            MeshPushConstants::PushConstant1 constants;
            constants.modelViewProjection = mvp;
            vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant1), &constants);

            vkCmdDraw(cmd, mesh.vertices.size(), 1, 0, 0);
        });
    }

    ~Ex02VertexBufferInput() {
        vkFreeCommandBuffers(vc.GetDevice(), frameSyncCmd.commandPool, 1, &frameSyncCmd.mainCommandBuffer);
    }
private:
    VkRenderPass renderPass;
    FrameSyncCmd frameSyncCmd;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkFramebuffer> presentFramebuffers;

    Mesh triangleMesh, quadMesh, monkeyMesh;
};