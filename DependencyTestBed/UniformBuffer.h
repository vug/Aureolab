#include "Utilities.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <vector>

namespace UniformBuffer {
    static const char* vertex_shader_text =
        "#version 460 core\n"
        "uniform mat4 uMVP;\n"
        "layout (location = 0) in vec3 vPos;\n"
        "layout (location = 1) in vec2 vUV;\n"
        "out vec2 uv;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = uMVP * vec4(vPos, 1.0);\n"
        "   uv = vUV;\n"
        "}\n";

    static const char* fragment_shader_text =
        "#version 460 core\n"
        "#define MAX_DISKS 10\n"
        "struct Disk {\n"
        "    vec2 location;\n"
        "    float radius;\n"
        " };\n"
        "layout(binding = 0, std140) uniform DisksBlock {\n"
        "    Disk disks[MAX_DISKS];\n"
        "};\n"
        "uniform int uNumDisks\n;"
        "in vec2 uv;\n"
        "out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "   vec2 p = (uv - 0.5) * 2.0;\n"
        "   outColor = vec4(vec3(0.0), 1.0);\n"
        "   for (int i = 0; i < uNumDisks; i++) {\n"
        "       float r = length(p - disks[i].location);\n"
        "       float disk = 1.0 - smoothstep(disks[i].radius - 0.005, disks[i].radius, r);\n"
        "       outColor += vec4(vec3(disk), 1.0);\n"
        "   }\n"
        "}\n";

    struct Vertex {
        glm::vec3 position = {};
        glm::vec2 uv = {};
    };
    struct Disk {
        glm::vec2 location;
        alignas(8) float radius;
        // OR
        //float radius;
        //float _pad1;
    };

    int Main() {
        auto LOGGER = Utils::Log::CreateLogger("UBOs");

        Utils::GL::InitGLFWLoadGLFunctions();
        GLFWwindow* window = glfwGetCurrentContext();

        auto shader1 = Utils::GL::MakeShaderProgram(vertex_shader_text, fragment_shader_text);
        GLuint uMVPLocation;
        uMVPLocation = glGetUniformLocation(shader1, "uMVP");

        GLuint disksBlockIndexForShader1;
        disksBlockIndexForShader1 = glGetUniformBlockIndex(shader1, "DisksBlock");
        GLuint disksBindingPoint = 0; // Binding point values are stored by the app manually. Multiple shaders can bind to this point.
        glUniformBlockBinding(shader1, disksBlockIndexForShader1, disksBindingPoint);

        GLuint uNumDisksLocation;
        uNumDisksLocation = glGetUniformLocation(shader1, "uNumDisks");

        GLuint vPosLocation, vUvLocation;
        vPosLocation = glGetAttribLocation(shader1, "vPos");
        vUvLocation = glGetAttribLocation(shader1, "vUV");

        Vertex vertices[] = {
            { { -1.0, -1.0, 0.0 }, { 0.0, 0.0 } },
            { { +1.0, -1.0, 0.0 }, { 1.0, 0.0 } },
            { { -1.0, +1.0, 0.0 }, { 0.0, 1.0 } },
            { { +1.0, +1.0, 0.0 }, { 1.0, 1.0 } },
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

        int uNumDisks = 5;
        GLuint ubo;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, uNumDisks * sizeof(Disk), NULL, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, disksBlockIndexForShader1, ubo);

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

            glUseProgram(shader1);
            glm::mat4 uMVP = glm::mat4(1.0);
            glUniformMatrix4fv(uMVPLocation, 1, GL_FALSE, glm::value_ptr(uMVP));
            glUniform1i(uNumDisksLocation, uNumDisks);
            std::vector<Disk> disks;
            for (int i = 0; i < uNumDisks; i++) {
                float angle = i * Utils::Math::TAU / uNumDisks + glfwGetTime();
                disks.emplace_back(Disk { { 0.5f * cosf(angle), 0.5f * sinf(angle) }, 0.1f * (float)i / uNumDisks + 0.01f });
            }
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferData(GL_UNIFORM_BUFFER, uNumDisks * sizeof(Disk), disks.data(), GL_DYNAMIC_DRAW);

            //glBindVertexArray(vao);
            //glBindBuffer(GL_ARRAY_BUFFER, vbo);
            //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

            Utils::ImGUI::Render();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glDeleteProgram(shader1);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        Utils::ImGUI::Shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }
}
