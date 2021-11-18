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

    while (!win.ShouldClose()) {
        win.PollEvents();
    }

    vkDestroyRenderPass(vc.GetDevice(), renderPass, nullptr);
    return 0;
}