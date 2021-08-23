#include "Core/Log.h"
#include "Core/Window.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

OpenGLContext::OpenGLContext(Window* window) {
    // GLFW couples Window and Context. Graphics context was actually generated when glfwCreateWindow was called. Just retrieve that one.
    context = (GLFWwindow*)window->GetNativeWindow();
    // Make that context active so that GL calls happen there
    glfwMakeContextCurrent(context);

    // Load OpenGL and extension functions
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    SetVSync(false);

    Log::Info("OpenGL Context has been initialized. Info:");
    Log::Info("Renderer: {}", glGetString(GL_RENDERER));
    Log::Info("Vendor: {}", glGetString(GL_VENDOR));
    Log::Info("Version: {}", glGetString(GL_VERSION));
}

void OpenGLContext::OnUpdate() {
    SwapBuffers();
}

void OpenGLContext::SwapBuffers() {
	glfwSwapBuffers(context);
}

void OpenGLContext::SetVSync(bool toggle) {
    glfwSwapInterval(toggle);
}
