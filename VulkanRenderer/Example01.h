#pragma once

#include "Example.h"

class Ex01NoVertexInput : public Example {
public:
    Ex01NoVertexInput(VulkanContext& vc, VulkanRenderer& vr) :
        Example(vc, vr) {

        Mesh triangleMesh;
        triangleMesh.vertices = {
            { {  0.0f, -0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 1.f, 0.f, 0.0f, 1.0f } },
            { {  0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 1.f, 0.0f, 1.0f } },
            { { -0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 0.0f}, { 0.f, 0.f, 1.0f, 1.0f } },
        };
        vr.UploadMesh(triangleMesh);
        destroyer.Add(triangleMesh.vertexBuffer);
        Mesh quadMesh;
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

        VkShaderModule vertShader1 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-triangle-vert.spv"));
        VkShaderModule fragShader1 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-triangle-frag.spv"));
        destroyer.Add(vertShader1); destroyer.Add(fragShader1);
        VkPipelineLayout pipelineLayout1;
        std::tie(pipeline1, pipelineLayout1) = vr.CreateSinglePassGraphicsPipeline(vertShader1, fragShader1, renderPass);
        destroyer.Add(pipelineLayout1);
        destroyer.Add(pipeline1);

        VkShaderModule vertShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-vert.spv"));
        VkShaderModule fragShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-frag.spv"));
        destroyer.Add(vertShader2);
        destroyer.Add(fragShader2);
        VkPipelineLayout pipelineLayout2;
        std::tie(pipeline2, pipelineLayout2) = vr.CreateSinglePassGraphicsPipeline(vertShader2, fragShader2, renderPass);
        destroyer.Add(pipelineLayout2);
        destroyer.Add(pipeline2);

        cmdBuf = vr.CreateCommandBuffer();
    }

    void OnRender() {
        VkClearValue clearValue;
        static int frameNumber = 0;
        float flash = abs(sin(frameNumber / 2400.f));
        clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };
        vc.drawFrameBlocked(renderPass, cmdBuf, presentFramebuffers, vc.GetSwapchainInfo(), clearValue, [&](VkCommandBuffer& cmd) {
            if (int(frameNumber / 4000.0f) % 2 == 0) {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline1);
                vkCmdDraw(cmd, 3, 1, 0, 0);
            }
            else {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline2);
                vkCmdDraw(cmd, 6, 1, 0, 0);
            }
        });

        frameNumber++;
    }

    ~Ex01NoVertexInput() {
        vkFreeCommandBuffers(vc.GetDevice(), vc.GetCommandPool(), 1, &cmdBuf);
    }
private:
    VkRenderPass renderPass;
    VkPipeline pipeline1, pipeline2;
    VkCommandBuffer cmdBuf;
    std::vector<VkFramebuffer> presentFramebuffers;
};