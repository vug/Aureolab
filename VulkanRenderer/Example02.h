#pragma once

#include "Example.h"

class Ex02VertexBufferInput : public Example {
public:
    Ex02VertexBufferInput(VulkanContext& vc, VulkanRenderer& vr) :
        Example(vc, vr) {
        triangleMesh.vertices = {
            { {  0.0f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.f, 0.f, 0.0f, 1.0f } },
            { {  0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 1.f, 0.0f, 1.0f } },
            { { -0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 0.f, 1.0f, 1.0f } },
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

        renderPass = vr.CreateRenderPass();
        destroyer.Add(renderPass);
        presentFramebuffers = vc.CreateSwapChainFrameBuffers(vc.GetDevice(), renderPass, vc.GetSwapchainInfo());
        destroyer.Add(presentFramebuffers);

        VkShaderModule vertShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-vertex-attr-vert.spv"));
        VkShaderModule fragShader = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-vertex-attr-frag.spv"));
        destroyer.Add(vertShader);
        destroyer.Add(fragShader);
        VkPipelineLayout pipelineLayout;
        std::tie(pipeline, pipelineLayout) = vr.CreateSinglePassGraphicsPipeline(vertShader, fragShader, Vertex::GetVertexDescription(), renderPass);
        destroyer.Add(pipelineLayout);
        destroyer.Add(pipeline);

        cmdBuf = vr.CreateCommandBuffer();
    }

    void OnRender() {
        static int frameNumber = 0;

        VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
        vc.drawFrameBlocked(renderPass, cmdBuf, presentFramebuffers, vc.GetSwapchainInfo(), clearValue, [&](VkCommandBuffer& cmd) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            Mesh mesh = int(frameNumber / 2000.0f) % 2 == 0 ? triangleMesh : quadMesh;

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);

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
    VkCommandBuffer cmdBuf;
    std::vector<VkFramebuffer> presentFramebuffers;

    Mesh triangleMesh, quadMesh;
};