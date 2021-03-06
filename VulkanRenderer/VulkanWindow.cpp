#include "VulkanWindow.h"

#include "Core/Log.h"

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    void* ptr = glfwGetWindowUserPointer(window);
    if (ptr == nullptr) {
        return;
    }
    // can cause an error if object set via SetUserPointer is not IResizable
    auto resizable = reinterpret_cast<IResizable*>(ptr);
    resizable->OnResize(width, height);
}

VulkanWindow::VulkanWindow() {
    Log::Debug("Constructing GLFW Window...");
	int WIDTH = 1024;
	int HEIGHT = 768;

    if (!glfwInit()) {
        Log::Critical("Could not initialize GLFW!");
        exit(EXIT_FAILURE);
    }

    // Do not create OpenGL context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // True is the default value of resizability hint
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan App", nullptr, nullptr);
    if (window == nullptr) {
        Log::Critical("Could not create GLFW window!");
        exit(EXIT_FAILURE);
    }

    glfwVulkanExtensions = glfwGetRequiredInstanceExtensions(&glfwVulkanExtensionCount);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

VulkanWindow::~VulkanWindow() {
    Log::Debug("Destructing GLFW Window...");
    glfwDestroyWindow(window);

    glfwTerminate();
}

void VulkanWindow::GetFramebufferSize(int* width, int* height) const {
    glfwGetFramebufferSize(window, width, height);
}

VulkanWindow::Vec2D VulkanWindow::GetMouseCursorPosition() {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return { float(x), float(y) };
}

bool VulkanWindow::ShouldClose() {
    return glfwWindowShouldClose(window);
}

void VulkanWindow::PollEvents() {
    glfwPollEvents();
}

// ptr can be referred in GLFW's static callback functions
void VulkanWindow::SetUserPointer(void* ptr) {
    glfwSetWindowUserPointer(window, ptr);
}

VkResult VulkanWindow::CreateSurface(VkInstance instance, VkSurfaceKHR* surface) const {
    return glfwCreateWindowSurface(instance, window, nullptr, surface);
}
