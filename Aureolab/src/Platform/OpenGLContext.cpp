#include "Core/Log.h"
#include "Core/Window.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <string>

inline const char* glMessageSourceToString(GLenum source) {
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        return "OpenGL API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "Shader compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "Third-party app associated with OpenGL";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "The user of this application";
    case GL_DEBUG_SOURCE_OTHER:
        return "Unspecified";
    default:
        assert(false); // unknown source
    }
};

inline const char* glMessageTypeToString(GLenum type) {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "Deprecated behavior";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "Undefined behavior";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "Unportable functionality";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "Performance issue";
    case GL_DEBUG_TYPE_MARKER:
        return "Command stream annotation";
    case GL_DEBUG_TYPE_PUSH_GROUP:
        return "Group pushing";
    case GL_DEBUG_TYPE_POP_GROUP:
        return "Group popping";
    case GL_DEBUG_TYPE_OTHER:
        return "Unspecified";
    default:
        assert(false); // unknown type
    }
}

inline const char* glMessageSeverityToString(GLenum severity) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        return "High";
    case GL_DEBUG_SEVERITY_MEDIUM:
        return "Medium";
    case GL_DEBUG_SEVERITY_LOW:
        return "Low";
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "Notification";
    }
}

void GLAPIENTRY OpenGLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const char* message, const void* userParam) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        Log::Critical("OpenGL Debug Message [{}]. Severity: {}. Source: {}. Type: {}. Message: {}.",
            id, glMessageSeverityToString(severity), glMessageSourceToString(source), glMessageTypeToString(type), message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        Log::Error("OpenGL Debug Message [{}]. Severity: {}. Source: {}. Type: {}. Message: {}.",
            id, glMessageSeverityToString(severity), glMessageSourceToString(source), glMessageTypeToString(type), message);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        Log::Warning("OpenGL Debug Message [{}]. Severity: {}. Source: {}. Type: {}. Message: {}.",
            id, glMessageSeverityToString(severity), glMessageSourceToString(source), glMessageTypeToString(type), message);
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        Log::Trace("OpenGL Debug Message [{}]. Severity: {}. Source: {}. Type: {}. Message: {}.",
            id, glMessageSeverityToString(severity), glMessageSourceToString(source), glMessageTypeToString(type), message);
        break;
    default:
        assert(false); // unknown severity
    }
}

OpenGLContext::OpenGLContext(Window* window) {
    // GLFW couples Window and Context. Graphics context was actually generated when glfwCreateWindow was called. Just retrieve that one.
    context = (GLFWwindow*)window->GetNativeWindow();
    // Make that context active so that GL calls happen there
    glfwMakeContextCurrent(context);

    // Load OpenGL and extension functions
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    SetVSync(false);

#ifdef AL_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLDebugMessageCallback, nullptr);
    // Ignore notifications
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

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
