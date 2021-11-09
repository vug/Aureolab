#include "Core/Log.h"

#include "Window.h"
#include "VulkanRenderer.h"

int main() {
    Window win;
    VulkanRenderer vr = { win };

    while (!win.ShouldClose()) {
        win.PollEvents();
    }
    return 0;
}