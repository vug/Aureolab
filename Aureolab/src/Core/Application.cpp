#include "Application.h"
#include "Log.h"

#include <string>

static void error_callback(int error, const char* description)
{
    Log::Error("Error [{}]: {}", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = (Application*)glfwGetWindowUserPointer(window);
    app->OnKeyPress(key, scancode, action, mods);
}

Application::Application(const std::string& name) : name(name) { 
    // Window::Initialize
    if (!glfwInit()) {
        Log::Critical("Could not initialize GLFW. Exiting...");
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef IS_MAC_OS // find an actual macro to detect MacOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    // other hints: GLFW_OPENGL_DEBUG_CONTEXT, GLFW_COCOA_RETINA_FRAMEBUFFER 

    window = glfwCreateWindow(1000, 1000, name.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        Log::Critical("Could not initialize GLFW window. Exiting...");
        return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, key_callback);
    glfwSetErrorCallback(error_callback);

    {
        // GraphicsContext::Initialize
        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    }

    glfwSwapInterval(1);  // VSync enabled. (0 for disabling), requires context

    Log::Info("OpenGL Info:");
    Log::Info("Renderer: {}", glGetString(GL_RENDERER));
    Log::Info("Vendor: {}", glGetString(GL_VENDOR));
    Log::Info("Version: {}", glGetString(GL_VERSION));

    // Renderer::Initialize
    // TODO: Enable GL debugging, other glEnable (blending, blend function, depth test etc) to application defaults
}

void Application::Run() {
	Log::Info("{} is running. Creating window...", name);

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        for (auto layer : layers) {
            layer->OnUpdate(0.1f);
        }

        // Window::OnUpdate
        glfwPollEvents();
        {
            // GraphicsContext
            glfwSwapBuffers(window);
        }
    }

    for (int i = 0; i < layers.size(); i++) {
        PopLayer();
    }

    // Window::Shutdown
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::Close() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Application::OnKeyPress(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        Close();
}