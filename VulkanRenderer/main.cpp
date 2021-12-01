#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"
#include "Example.h"
#include "Example01.h"
#include "Example02.h"
#include "Example03.h"


int main() {
    VulkanWindow win;
    VulkanContext vc = { win };
    VulkanRenderer vr = { vc };
;
    auto ex01 = Ex01NoVertexInput(vc, vr);
    auto ex02 = Ex02VertexBufferInput(vc, vr);
    auto ex03 = Ex03SceneManagement(vc, vr);
    Example& example = ex03;

    while (!win.ShouldClose()) {
        win.PollEvents();

        example.OnRender();
    }

    // drawFrame operations are async.
    // should finish them before starting cleanup while leaving the main loop
    vkDeviceWaitIdle(vc.GetDevice());

    return 0;
}