#include "WindowsWindow.h"

#include "Core/Log.h"
#include "Events/WindowEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>


static void error_callback(int error, const char* description) {
    Log::Error("Error [{}]: {}", error, description);
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

    // GLFW Callbacks
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        auto ev = WindowResizeEvent(width, height);
        ptr.Dispatch(ev);
    });

    glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        auto ev = WindowCloseEvent();
        ptr.Dispatch(ev);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        switch (action) {
        case GLFW_PRESS: {
            auto ev = KeyPressedEvent(key, false);
            ptr.Dispatch(ev);
            break;
        }
        case GLFW_RELEASE:{
            auto ev = KeyReleasedEvent(key);
            ptr.Dispatch(ev);
            break;
        }
        case GLFW_REPEAT: {
            auto ev = KeyPressedEvent(key, true);
            ptr.Dispatch(ev);
            break;
        }
        }
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        switch (action) {
        case GLFW_PRESS: {
            auto ev = MouseButtonPressedEvent(button);
            ptr.Dispatch(ev);
            break;
        }
        case GLFW_RELEASE: {
            auto ev = MouseButtonReleasedEvent(button);
            ptr.Dispatch(ev);
            break;
        }
        }
    });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        auto ev = MouseScrolledEvent((float)xOffset, (float)yOffset);
        ptr.Dispatch(ev);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        auto ev = MouseMovedEvent((float)xPos, (float)yPos);
        ptr.Dispatch(ev);
    });


    glfwSetFramebufferSizeCallback(window, [](GLFWwindow * window, int width, int height) {
        UserPointer& ptr = *(UserPointer*)glfwGetWindowUserPointer(window);
        auto ev = FrameBufferResizeEvent((unsigned int)width, (unsigned int)height);
        ptr.Dispatch(ev);
    });

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