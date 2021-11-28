#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"
#include "VulkanDestroyer.h"
#include "Mesh.h"
#include "Example.h"
#include "Example01.h"

#include <tuple>


int main() {
    VulkanWindow win;
    VulkanContext vc = { win };
    VulkanRenderer vr = { vc };
;
    auto example = Ex01NoVertexInput(vc, vr);

    while (!win.ShouldClose()) {
        win.PollEvents();

        example.OnRender();
    }

    // drawFrame operations are async.
    // should finish them before starting cleanup while leaving the main loop
    vkDeviceWaitIdle(vc.GetDevice());

    return 0;
}