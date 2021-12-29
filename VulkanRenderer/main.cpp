#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"
//#include "Examples/Example01.h"
//#include "Examples/Example02.h"
//#include "Examples/Example03.h"
//#include "Examples/Example04.h"
//#include "Examples/Example05.h"
#include "Examples/Example06.h"

int main() {
    VulkanWindow win;
    for (uint32_t i = 0; i < win.GetVulkanExtensionCount(); i++)
        Log::Info("{}", win.GetVulkanExtensions()[i]);
    VulkanContext vc = { win };
    VulkanRenderer vr = { vc };

    //auto ex01 = Ex01NoVertexInput(vc, vr);
    //auto ex02 = Ex02VertexBufferInput(vc, vr);
    //auto ex03 = Ex03SceneManagement(vc, vr);
    //auto ex04 = Ex04DescriptorSets(vc, vr);
    //auto ex05 = Ex05Textures(vc, vr);
    auto ex06 = Ex06Plain(vc, vr);
    Example& example = ex06;

    while (!win.ShouldClose()) {
        win.PollEvents();

        example.OnRender();
    }

    // drawFrame operations are async.
    // should finish them before starting cleanup while leaving the main loop
    vkDeviceWaitIdle(vc.GetDevice());

    return 0;
}
