#pragma once

#include "Example.h"

#include <glm/gtx/transform.hpp>

class Ex02VertexBufferInput : public Example {
public:
    Ex02VertexBufferInput(VulkanContext& vc, VulkanRenderer& vr) :
        Example(vc, vr) {
        triangleMesh.vertices = {
            { { -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.f, 0.f, 0.0f, 1.0f } },
            { {  0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 1.f, 0.0f, 1.0f } },
            { {  0.0f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 0.f, 1.0f, 1.0f } },
        };
        vr.UploadMesh(triangleMesh);
        destroyer.Add(triangleMesh.vertexBuffer);
        quadMesh.vertices = {
            { { -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.f, 0.f, 0.0f, 1.0f } },
            { {  0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 1.f, 0.0f, 1.0f } },
            { {  0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 0.f, 1.0f, 1.0f } },

            { { -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.f, 0.f, 0.0f, 1.0f } },
            { {  0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 0.f, 1.0f, 1.0f } },
            { { -0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.f, 1.f, 1.0f, 1.0f } },
        };
        vr.UploadMesh(quadMesh);
        destroyer.Add(quadMesh.vertexBuffer);
        monkeyMesh.LoadFromOBJ("assets/models/suzanne.obj");
        vr.UploadMesh(monkeyMesh);
        destroyer.Add(monkeyMesh.vertexBuffer);

        renderPass = vr.CreateRenderPass();
        destroyer.Add(renderPass);
        presentFramebuffers = vc.CreateSwapChainFrameBuffers(vc.GetDevice(), renderPass, vc.GetSwapchainInfo());
        destroyer.Add(presentFramebuffers);

        VkShaderModule vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-push-const-vert.spv"));
        VkShaderModule fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-push-const-frag.spv"));
        destroyer.Add(vertShader);
        destroyer.Add(fragShader);
        std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), MeshPushConstants::GetPushConstantRanges(), renderPass);
        destroyer.Add(pipelineLayout);
        destroyer.Add(pipeline);

        cmdBuf = vr.CreateCommandBuffer();
    }

    void OnRender() {
        static int frameNumber = 0;

        VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
        vc.drawFrameBlocked(renderPass, cmdBuf, presentFramebuffers, vc.GetSwapchainInfo(), clearValue, [&](VkCommandBuffer& cmd) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            Mesh mesh = int(frameNumber / 3000.0f) % 2 == 0 ? monkeyMesh : triangleMesh;

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);

            glm::vec3 camPos = { 0.f, 0.f, -2.f };
            glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
            glm::mat4 projection = glm::perspective(glm::radians(70.f), 800.f / 600.f, 0.1f, 200.0f);
            projection[1][1] *= -1;
            glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(frameNumber * 0.1f), glm::vec3(0, 1, 0));
            glm::mat4 mvp = projection * view * model;
            MeshPushConstants::PushConstant1 constants;
            constants.modelViewProjection = mvp;
            vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants::PushConstant1), &constants);

            vkCmdDraw(cmd, mesh.vertices.size(), 1, 0, 0);
        });

        frameNumber++;
    }

    ~Ex02VertexBufferInput() {
        vkFreeCommandBuffers(vc.GetDevice(), vc.GetCommandPool(), 1, &cmdBuf);
    }
private:
    VkRenderPass renderPass;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkCommandBuffer cmdBuf;
    std::vector<VkFramebuffer> presentFramebuffers;

    Mesh triangleMesh, quadMesh, monkeyMesh;
};