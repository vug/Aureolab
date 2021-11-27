#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"
#include "VulkanDestroyer.h"

int main() {
    VulkanWindow win;
    VulkanContext vc = { win };
    VulkanRenderer vr = { vc };

    VulkanDestroyer destroyer{ vc.GetDevice() };

    VkCommandBuffer cmdBuf = vr.CreateCommandBuffer();
    VkRenderPass renderPass = vr.CreateRenderPass();
    destroyer.Add(renderPass);
    auto presentFramebuffers = vc.CreateSwapChainFrameBuffers(vc.GetDevice(), renderPass, vc.GetSwapchainInfo());
    destroyer.Add(presentFramebuffers);

    VkShaderModule vertShader1 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-triangle-vert.spv"));
    VkShaderModule fragShader1 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-triangle-frag.spv"));
    destroyer.Add(vertShader1); destroyer.Add(fragShader1);
    auto [pipeline1, pipelineLayout1] = vr.CreateSinglePassGraphicsPipeline(vertShader1, fragShader1, renderPass);
    destroyer.Add(pipelineLayout1); destroyer.Add(pipeline1);

    VkShaderModule vertShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-vert.spv"));
    VkShaderModule fragShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-frag.spv"));
    destroyer.Add(vertShader2); destroyer.Add(fragShader2);
    auto [pipeline2, pipelineLayout2] = vr.CreateSinglePassGraphicsPipeline(vertShader2, fragShader2, renderPass);
    destroyer.Add(pipelineLayout2); destroyer.Add(pipeline2);

    while (!win.ShouldClose()) {
        win.PollEvents();

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

    // drawFrame operations are async.
    // should finish them before starting cleanup while leaving the main loop
    vkDeviceWaitIdle(vc.GetDevice());

    vkFreeCommandBuffers(vc.GetDevice(), vc.GetCommandPool(), 1, &cmdBuf);

    destroyer.DestroyAll();
    return 0;
}