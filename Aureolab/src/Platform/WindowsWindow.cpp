#include "Core/Log.h"
#include "WindowsWindow.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>


static void error_callback(int error, const char* description) {
    Log::Error("Error [{}]: {}", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void WindowsWindow::Initialize(const std::string& name, int width, int height) {
    if (!glfwInit()) {
        Log::Critical("Could not initialize GLFW. Exiting...");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // other hints: GLFW_OPENGL_DEBUG_CONTEXT, GLFW_COCOA_RETINA_FRAMEBUFFER 

    window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        Log::Critical("Could not initialize GLFW window. Exiting...");
        return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, key_callback);
    glfwSetErrorCallback(error_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    {
        // GraphicsContext::Initialize
        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        Log::Info("OpenGL Info:");
        Log::Info("Renderer: {}", glGetString(GL_RENDERER));
        Log::Info("Vendor: {}", glGetString(GL_VENDOR));
        Log::Info("Version: {}", glGetString(GL_VERSION));
    }

    glfwSwapInterval(1);  // VSync enabled. (0 for disabling), requires context
    Log::Info("GLFW Window and graphics context are initialized");
}

void WindowsWindow::OnUpdate() {
    glfwPollEvents();
    {
        // GraphicsContext
        glfwSwapBuffers(window);
    }
}

bool WindowsWindow::IsRunning() {
    return !glfwWindowShouldClose(window);
}

void WindowsWindow::Shutdown() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

int WindowsWindow::GetWidth() {
    int w;
    glfwGetWindowSize(window, &w, nullptr);
    return w;
}

int WindowsWindow::GetHeight() {
    int h;
    glfwGetWindowSize(window, nullptr, &h);
    return h;
}
