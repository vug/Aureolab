#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"

int main() {
    VulkanWindow win;
    VulkanContext vc = { win };

    while (!win.ShouldClose()) {
        win.PollEvents();
    }
    return 0;
}