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

    while (!win.ShouldClose()) {
        win.PollEvents();

        vc.drawFrame(renderPass, cmdBuf, presentFramebuffers, vc.GetSwapchainInfo());
    }

    // drawFrame operations are async.
    // should finish them before starting cleanup while leaving the main loop
    vkDeviceWaitIdle(vc.GetDevice());

    vkFreeCommandBuffers(vc.GetDevice(), vc.GetCommandPool(), 1, &cmdBuf);
    for (size_t i = 0; i < presentImageViews.size(); i++) {
        vkDestroyFramebuffer(vc.GetDevice(), presentFramebuffers[i], nullptr);
    }
    vkDestroyRenderPass(vc.GetDevice(), renderPass, nullptr);
    return 0;
}