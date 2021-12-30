#include "Core/Log.h"

#include "VulkanWindow.h"
#include "VulkanContext.h"
#include "VulkanRenderer.h"
//#include "Examples/Example01.h"
//#include "Examples/Example02.h"
//#include "Examples/Example03.h"
//#include "Examples/Example04.h"
//#include "Examples/Example05.h"
//#include "Examples/Example06.h"
#include "Examples/Example07.h"

class Timer {
public:
    struct Time {
        float time = {};
        float delta = {};
    };
    Timer() :
        t0(std::chrono::system_clock::now()),
        prevTime(std::chrono::duration<float>(0.0f)) {
    }

    Time read() {
        std::chrono::duration<float> time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - t0);
        std::chrono::duration<float> delta = time - prevTime;
        prevTime = time;
        return { time.count(), delta.count() };
    }
private:
    std::chrono::system_clock::time_point t0;
    std::chrono::duration<float> prevTime;
};

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
    //auto ex06 = Ex06SplitScreen(vc, vr);
    auto ex07 = Ex07Plain(vc, vr);
    Example& example = ex07;

    Timer timer;

    while (!win.ShouldClose()) {
        auto [time, delta] = timer.read();
        win.PollEvents();

        example.OnRender(time, delta);
    }

    // drawFrame operations are async.
    // should finish them before starting cleanup while leaving the main loop
    vkDeviceWaitIdle(vc.GetDevice());

    return 0;
}
