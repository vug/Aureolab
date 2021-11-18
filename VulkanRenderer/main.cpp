#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"

int main() {
    VulkanWindow win;
    VulkanContext vc = { win };

    VkCommandPool cmdPool = vc.GetCommandPool();
    VulkanRenderer vr = { vc };

    Log::Debug("Creating main, graphics Command Buffer...");
    VkCommandBuffer cmdBuf;
	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.pNext = nullptr;
	cmdAllocInfo.commandPool = cmdPool;
	cmdAllocInfo.commandBufferCount = 1;
    // Primary commands are directly sent to queues. Secondary ones are subcommands (used in multi-threaded scenarios)
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    if (vkAllocateCommandBuffers(vc.GetDevice(), &cmdAllocInfo, &cmdBuf) != VK_SUCCESS) {
        Log::Critical("Failed to create main command buffer!");
        exit(EXIT_FAILURE);
    }

    while (!win.ShouldClose()) {
        win.PollEvents();
    }

    return 0;
}