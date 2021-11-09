#include "Core/Log.h"

#include "Window.h"
#include "VulkanRenderer.h"

int main() {
    Window win;
    VulkanRenderer vr = { win };

    return 0;
}