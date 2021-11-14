#include "Core/Log.h"

#include "Window.h"
#include "VulkanRenderer.h"

int main() {
    Window win;
    VulkanRenderer vr = { win };
    vr.CreateExampleGraphicsPipeline("assets/shaders/example-vert.spv", "assets/shaders/example-frag.spv");

    while (!win.ShouldClose()) {
        win.PollEvents();
    }
    return 0;
}