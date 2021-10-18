#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Utils {
    namespace Log {
        /*
         * Usage:
         * auto LOGGER = CreateLogger();
         * LOGGER->info("texture renderer ID: {}", colorRendererID);
         */
        std::shared_ptr<spdlog::logger> CreateLogger(const char* loggerName) {
            std::vector<spdlog::sink_ptr> sinks = {
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            };
            sinks[0]->set_pattern("%^[%T.%e] %n: %v%$");
            auto logger = std::make_shared<spdlog::logger>(loggerName, begin(sinks), end(sinks));
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::trace);
            return logger;
        }
    }
}

auto UtilsLogger = Utils::Log::CreateLogger("UtilsLogger");

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Utils {
    class GL {
    public:
        static void InitGLFWLoadGLFunctions() {
            glfwSetErrorCallback(glfwErrorCallback);

            if (!glfwInit()) {
                UtilsLogger->critical("Could not initialize GLFW. Exiting...");
                exit(1);
            }

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

            GLFWwindow* window = glfwCreateWindow(1024, 768, "Dependency Test Bed", NULL, NULL);
            if (!window) {
                glfwTerminate();
                UtilsLogger->critical("Could not initialize GLFW window. Exiting...");
                exit(1);
            }
            glfwMakeContextCurrent(window);
            gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
            glfwSwapInterval(0);  // VSync {0: disabled, 1: enabled}
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(OpenGLDebugMessageCallback, nullptr);
            // Ignore notifications
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

            //GLint glContextFlags;
            //glGetIntegerv(GL_CONTEXT_FLAGS, &glContextFlags);
            //GL_CONTEXT_PROFILE_MASK;
            UtilsLogger->info("OpenGL Info (glGetString):");
            UtilsLogger->info("Renderer: {}", glGetString(GL_RENDERER));
            UtilsLogger->info("Vendor: {}", glGetString(GL_VENDOR));
            UtilsLogger->info("Version: {}", glGetString(GL_VERSION));
            UtilsLogger->info("GLSL Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));
            //UtilsLogger->info("GL_CONTEXT_FLAGS: {}", glContextFlags);
        }

        static GLuint MakeShaderProgram(const char* vertex_shader_text, const char* fragment_shader_text) {
            GLuint vertex_shader, fragment_shader, program;

            vertex_shader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
            glCompileShader(vertex_shader);

            fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
            glCompileShader(fragment_shader);

            program = glCreateProgram();
            glAttachShader(program, vertex_shader);
            glAttachShader(program, fragment_shader);
            glLinkProgram(program);
            return program;
        }

    private:
        static inline const char* glMessageSourceToString(GLenum source) {
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
                return "Unknown";
            }
        };

        static inline const char* glMessageTypeToString(GLenum type) {
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
                return "Unknown";
            }
        }

        static inline const char* glMessageSeverityToString(GLenum severity) {
            switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
                return "High";
            case GL_DEBUG_SEVERITY_MEDIUM:
                return "Medium";
            case GL_DEBUG_SEVERITY_LOW:
                return "Low";
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                return "Notification";
            default:
                return "Unknown";
            }
        }

        static void GLAPIENTRY OpenGLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
            GLsizei length, const char* message, const void* userParam) {
            switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:
                UtilsLogger->critical("OpenGL Debug Message [{}]. Severity: {}. Source: {}. Type: {}. Message: {}.",
                    id, glMessageSeverityToString(severity), glMessageSourceToString(source), glMessageTypeToString(type), message);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
                UtilsLogger->error("OpenGL Debug Message [{}]. Severity: {}. Source: {}. Type: {}. Message: {}.",
                    id, glMessageSeverityToString(severity), glMessageSourceToString(source), glMessageTypeToString(type), message);
                break;
            case GL_DEBUG_SEVERITY_LOW:
                UtilsLogger->warn("OpenGL Debug Message [{}]. Severity: {}. Source: {}. Type: {}. Message: {}.",
                    id, glMessageSeverityToString(severity), glMessageSourceToString(source), glMessageTypeToString(type), message);
                break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                UtilsLogger->trace("OpenGL Debug Message [{}]. Severity: {}. Source: {}. Type: {}. Message: {}.",
                    id, glMessageSeverityToString(severity), glMessageSourceToString(source), glMessageTypeToString(type), message);
                break;
            default:
                assert(false); // unknown severity
            }
        }

        static void glfwErrorCallback(int error, const char* description) {
            UtilsLogger->error("Error [{}]: {}", error, description);
        }
    };
}

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace Utils {
    namespace ImGUI {
        void Init(GLFWwindow* window) {
            // ImGui Init
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
            // platform/renderer bindings
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 450");
            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }
        }

        void NewFrame() {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        void Render() {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            // Update and Render additional Platform Windows (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context); //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            }
        }

        void Shutdown() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }
}


