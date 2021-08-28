#include "WindowsWindow.h"

#include "Events/WindowEvent.h"
#include "Core/Log.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>


static void error_callback(int error, const char* description) {
    Log::Error("Error [{}]: {}", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

WindowsWindow::WindowsWindow(const std::string& name, int width, int height) 
    : window(nullptr) {
    if (!glfwInit()) {
        Log::Critical("Could not initialize GLFW. Exiting...");
        return;
    }

    // Graphics Context related hints.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // GLFW_OPENGL_ES_API
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef AL_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // TODO: removes deprecated features

    // Creates both a window and a graphics context.
    window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        Log::Critical("Could not initialize GLFW window. Exiting...");
        return;
    }

    glfwSetErrorCallback(error_callback);
    glfwSetWindowUserPointer(window, &userPointer);
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        WindowResizeEvent ev(width, height);
        ptr.Dispatch(ev);
    });
    glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        ptr.Dispatch(WindowCloseEvent());
    });
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        // TODO: handle other keys, and repetition
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    });
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Log::Info("GLFW Window has been initialized");
}

void WindowsWindow::OnUpdate() {
    glfwPollEvents();
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

float WindowsWindow::GetTime() {
    return (float)glfwGetTime();
}