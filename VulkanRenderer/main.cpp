#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"

int main() {
    VulkanWindow win;
    VulkanContext vc = { win };
    VulkanRenderer vr = { vc };

    VkCommandBuffer cmdBuf = vr.CreateCommandBuffer();
    VkRenderPass renderPass = vr.CreateRenderPass();
    const auto& presentImageViews = vc.GetSwapchainInfo().imageViews;
    std::vector<VkFramebuffer> presentFramebuffers(presentImageViews.size());
    for (size_t i = 0; i < presentImageViews.size(); i++) {
        presentFramebuffers[i] = vr.CreateFramebuffer(renderPass, presentImageViews[i], vc.GetSwapchainInfo().extent);
    }

    VkShaderModule vertShader1 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-triangle-vert.spv"));
    VkShaderModule fragShader1 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-triangle-frag.spv"));
    auto [pipeline1, pipelineLayout1] = vr.CreateSinglePassGraphicsPipeline(vertShader1, fragShader1, renderPass);

    VkShaderModule vertShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-vert.spv"));
    VkShaderModule fragShader2 = vr.CreateShaderModule(vr.ReadFile("assets/shaders/example-square-frag.spv"));
    auto [pipeline2, pipelineLayout2] = vr.CreateSinglePassGraphicsPipeline(vertShader2, fragShader2, renderPass);

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

    vkDestroyShaderModule(vc.GetDevice(), vertShader1, nullptr);
    vkDestroyShaderModule(vc.GetDevice(), fragShader1, nullptr);
    vkDestroyPipelineLayout(vc.GetDevice(), pipelineLayout1, nullptr);
    vkDestroyPipeline(vc.GetDevice(), pipeline1, nullptr);

    vkDestroyShaderModule(vc.GetDevice(), vertShader2, nullptr);
    vkDestroyShaderModule(vc.GetDevice(), fragShader2, nullptr);
    vkDestroyPipelineLayout(vc.GetDevice(), pipelineLayout2, nullptr);
    vkDestroyPipeline(vc.GetDevice(), pipeline2, nullptr);

    vkFreeCommandBuffers(vc.GetDevice(), vc.GetCommandPool(), 1, &cmdBuf);
    for (size_t i = 0; i < presentImageViews.size(); i++) {
        vkDestroyFramebuffer(vc.GetDevice(), presentFramebuffers[i], nullptr);
    }
    vkDestroyRenderPass(vc.GetDevice(), renderPass, nullptr);
    return 0;
}