#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"

int main() {
    VulkanWindow win;
    VulkanContext vc = { win };

    VulkanRenderer vr = { vc };
    VkCommandBuffer cmdBuf = vr.CreateCommandBuffer();

    while (!win.ShouldClose()) {
        win.PollEvents();
    }

    return 0;
}