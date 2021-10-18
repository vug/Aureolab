#include "Utilities.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace UniformBuffer {
    static const char* vertex_shader_text =
        "#version 460 core\n"
        "uniform mat4 MVP;\n"
        "layout (location = 0) in vec3 vPos;\n"
        "layout (location = 1) in vec2 vUV;\n"
        "layout (location = 2) in vec3 vCol;\n"
        "out vec2 uv;\n"
        "out vec3 color;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = MVP * vec4(vPos, 1.0);\n"
        "   uv = vUV;\n"
        "   color = vCol;\n"
        "}\n";

    static const char* fragment_shader_text =
        "#version 460 core\n"
        "in vec2 uv;\n"
        "in vec3 color;\n"
        "out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "   float checker = int(uv.x * 10) % 2 ^ int(uv.y * 10) % 2;\n"
        "   outColor = vec4(checker * color, 1.0);\n"
        "}\n";

    struct Vertex {
        glm::vec3 position = {};
        glm::vec2 uv = {};
        glm::vec3 color = {};
    };

    int Main() {
        auto LOGGER = Utils::Log::CreateLogger("UBOs");

        Utils::GL::InitGLFWLoadGLFunctions();
        GLFWwindow* window = glfwGetCurrentContext();

        auto shader = Utils::GL::MakeShaderProgram(vertex_shader_text, fragment_shader_text);
        GLuint uMVPLocation;
        uMVPLocation = glGetUniformLocation(shader, "MVP");
        GLuint vPosLocation, vUvLocation, vColLocation;
        vPosLocation = glGetAttribLocation(shader, "vPos");
        vUvLocation = glGetAttribLocation(shader, "vUV");
        vColLocation = glGetAttribLocation(shader, "vCol");

        Vertex vertices[] = {
            { { -0.5, -0.5, 0.0 }, { 0.0, 0.0 }, { 0.9, 0.2, 0.1 } },
            { { +0.5, -0.5, 0.0 }, { 1.0, 0.0 }, { 0.1, 0.9, 0.2 } },
            { { -0.5, +0.5, 0.0 }, { 0.0, 1.0 }, { 0.2, 0.1, 0.9 } },
            { { +0.5, +0.5, 0.0 }, { 1.0, 1.0 }, { 0.8, 0.8, 0.2 } },
        };
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 1,
        };

        GLuint vbo, vao, ebo;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(vPosLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(vPosLocation);
        glVertexAttribPointer(vUvLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glEnableVertexAttribArray(vUvLocation);
        glVertexAttribPointer(vColLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(vColLocation);

        Utils::ImGUI::Init(window, false);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        while (!glfwWindowShouldClose(window)) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Utils::ImGUI::NewFrame();
            ImGui::Begin("ImGUI Window");
            {
                static bool shouldShow = false;
                ImGui::Checkbox("show demo", &shouldShow);
                if (shouldShow) { ImGui::ShowDemoWindow(); }
            }
            ImGui::End();

            glUseProgram(shader);
            glm::mat4 uMVP = glm::mat4(1.0);
            glUniformMatrix4fv(uMVPLocation, 1, GL_FALSE, glm::value_ptr(uMVP));
            glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

            Utils::ImGUI::Render();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glDeleteProgram(shader);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        Utils::ImGUI::Shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
}
