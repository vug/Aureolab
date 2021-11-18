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

    auto vertShaderByteCode = vr.ReadFile("assets/shaders/example-vert.spv");
    auto fragShaderByteCode = vr.ReadFile("assets/shaders/example-frag.spv");
    VkShaderModule vertShaderModule = vr.CreateShaderModule(vertShaderByteCode);
    VkShaderModule fragShaderModule = vr.CreateShaderModule(fragShaderByteCode);

    while (!win.ShouldClose()) {
        win.PollEvents();

        VkClearValue clearValue;
        static int frameNumber = 0;
        float flash = abs(sin(frameNumber / 2400.f));
        clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };
        vc.drawFrameBlocked(renderPass, cmdBuf, presentFramebuffers, vc.GetSwapchainInfo(), clearValue);

        frameNumber++;
    }

    // drawFrame operations are async.
    // should finish them before starting cleanup while leaving the main loop
    vkDeviceWaitIdle(vc.GetDevice());

    vkDestroyShaderModule(vc.GetDevice(), vertShaderModule, nullptr);
    vkDestroyShaderModule(vc.GetDevice(), fragShaderModule, nullptr);
    vkFreeCommandBuffers(vc.GetDevice(), vc.GetCommandPool(), 1, &cmdBuf);
    for (size_t i = 0; i < presentImageViews.size(); i++) {
        vkDestroyFramebuffer(vc.GetDevice(), presentFramebuffers[i], nullptr);
    }
    vkDestroyRenderPass(vc.GetDevice(), renderPass, nullptr);
    return 0;
}