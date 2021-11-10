#include "Window.h"

#include "Core/Log.h"

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    void* ptr = glfwGetWindowUserPointer(window);
    if (ptr == nullptr) {
        return;
    }
    auto resizable = reinterpret_cast<IResizable*>(ptr);
    resizable->OnResize(width, height);
}

Window::Window() {
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

Window::~Window() {
    Log::Debug("Destructing GLFW Window...");
    glfwDestroyWindow(window);

    glfwTerminate();
}

void Window::GetFramebufferSize(int* width, int* height) {
    glfwGetFramebufferSize(window, width, height);
}

bool Window::ShouldClose() {
    return glfwWindowShouldClose(window);
}

void Window::PollEvents() {
    glfwPollEvents();
}

// ptr can be referred in GLFW's static callback functions
void Window::SetUserPointer(void* ptr) {
    glfwSetWindowUserPointer(window, ptr);
}

VkResult Window::CreateSurface(VkInstance instance, VkSurfaceKHR* surface) {
    return glfwCreateWindowSurface(instance, window, nullptr, surface);
}
