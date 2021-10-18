#include "Utilities.h"



namespace StartingExample {
    static const char* vertex_shader_text =
        "#version 460 core\n"
        "uniform mat4 MVP;\n"
        "layout (location = 0) in vec2 vPos;\n"
        "layout (location = 1) in vec3 vCol;\n"
        "varying vec3 color;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
        "    color = vCol;\n"
        "}\n";

    static const char* fragment_shader_text =
        "#version 460 core\n"
        "varying vec3 color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(color, 1.0);\n"
        "}\n";

	int Main() {
		auto LOGGER = Utils::Log::CreateLogger("UBOs");

        Utils::GL::InitGLFWLoadGLFunctions();
        GLFWwindow* window = glfwGetCurrentContext();
		
        auto shader = Utils::GL::MakeShaderProgram(vertex_shader_text, fragment_shader_text);
        LOGGER->info("shader program ID: {}", shader);

        Utils::ImGUI::Init(window);
        glClearColor(0.1, 0.1, 0.1, 1.0);
        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            Utils::ImGUI::NewFrame();

            ImGui::Begin("ImGUI Window");
            {
                static bool shouldShow = false;
                ImGui::Checkbox("show demo", &shouldShow);
                if (shouldShow) { ImGui::ShowDemoWindow(); }
            }
            ImGui::End();

            Utils::ImGUI::Render();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glDeleteProgram(shader);
        Utils::ImGUI::Shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
	}
}
