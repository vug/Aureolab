#pragma once

#include "Example.h"

class FrameData01 : public IFrameData {};

class Ex01NoVertexInput : public Example {
public:
    Ex01NoVertexInput(VulkanContext& vc, VulkanRenderer& vr) :
        Example(vc, vr) {

        renderPass = vr.CreateRenderPass();
        destroyer.Add(renderPass);
        AllocatedImage depthImage;
        VkImageView depthImageView;
        std::tie(presentFramebuffers, depthImageView, depthImage) = vc.CreateSwapChainFrameBuffers(vc.GetDevice(), vc.GetAllocator(), renderPass, vc.GetSwapchainInfo());
        destroyer.Add(presentFramebuffers);
        destroyer.Add(depthImageView);
        destroyer.Add(depthImage);

        for (int i = 0; i < 1; i++) {
            FrameSyncCmd syncCmd = vc.CreateFrameSyncCmd(vc.GetDevice(), vc.GetQueueFamilyIndices().graphicsFamily.value());
            FrameData01 frame{ syncCmd };
            frameSyncCmds.push_back(frame);
            destroyer.Add(syncCmd.commandPool);
            destroyer.Add(syncCmd.renderFence);
            destroyer.Add(std::vector{ syncCmd.presentSemaphore, syncCmd.renderSemaphore });
        }

        VkShaderModule vertShader1 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-triangle-vert.spv"));
        VkShaderModule fragShader1 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-triangle-frag.spv"));
        destroyer.Add(vertShader1); destroyer.Add(fragShader1);
        VkPipelineLayout pipelineLayout1;
        std::tie(pipeline1, pipelineLayout1) = vr.CreateSinglePassGraphicsPipeline(vertShader1, fragShader1, {}, {}, renderPass);
        destroyer.Add(pipelineLayout1);
        destroyer.Add(pipeline1);

        VkShaderModule vertShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-vert.spv"));
        VkShaderModule fragShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-frag.spv"));
        destroyer.Add(vertShader2);
        destroyer.Add(fragShader2);
        VkPipelineLayout pipelineLayout2;
        std::tie(pipeline2, pipelineLayout2) = vr.CreateSinglePassGraphicsPipeline(vertShader2, fragShader2, {}, {}, renderPass);
        destroyer.Add(pipelineLayout2);
        destroyer.Add(pipeline2);
    }

    void OnRender() {
        std::vector<VkClearValue> clearValues(2);
        static int frameNumber = 0;
        float flash = abs(sin(frameNumber / 2400.f));
        clearValues[0].color = { { 0.0f, 0.0f, flash, 1.0f } };
        clearValues[1].depthStencil.depth = 1.0f;
        vc.drawFrame(vc.GetDevice(), vc.GetSwapchain(), vc.GetGraphicsQueue(), renderPass, frameSyncCmds, presentFramebuffers, vc.GetSwapchainInfo(), clearValues, [&](const VkCommandBuffer& cmd) {
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
        for (auto& frameSyncCmd : frameSyncCmds) {
            vkFreeCommandBuffers(vc.GetDevice(), frameSyncCmd.GetFrameSyncCmdData().commandPool, 1, &frameSyncCmd.GetFrameSyncCmdData().mainCommandBuffer);
        }
    }
private:
    VkRenderPass renderPass;
    std::vector<IFrameData> frameSyncCmds;
    VkPipeline pipeline1, pipeline2;
    VkCommandBuffer cmdBuf;
    std::vector<VkFramebuffer> presentFramebuffers;
};