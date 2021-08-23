#include "Core/Log.h"
#include "Core/EntryPoint.h"
#include "Core/Application.h"
#include "Core/Layer.h"

#include <glad/glad.h>

#include <array>
#include <iostream>

class Layer1 : public Layer {
public:
    Layer1() : Layer("FirstLayer") { }

    void OnAttach() {
        static const struct
        {
            float x, y;
            float r, g, b;
        } vertices[] =
        {
            { -0.6f, -0.4f, 1.f, 0.f, 0.f },
            {  0.6f, -0.4f, 0.f, 1.f, 0.f },
            {   0.f,  0.6f, 0.f, 0.f, 1.f },
            { -0.3f,  0.8f, 1.f, 0.f, 1.f },
        };
        indices = {
            0, 1, 2,
            0, 2, 3,
        };

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

        GLuint vertex_shader, fragment_shader;

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


        GLint vpos_location, vcol_location;

        mvp_location = glGetUniformLocation(program, "MVP");
        vpos_location = glGetAttribLocation(program, "vPos");
        vcol_location = glGetAttribLocation(program, "vCol");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
        glEnableVertexAttribArray(vpos_location);

        glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(vcol_location);
    }

    void OnUpdate(float ts) {
        GLfloat MVP[] = { // identity
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
        };

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)MVP);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
    }

    void OnDetach() {
        // optional
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteProgram(program);
    }

private:
    GLuint vbo = -1, vao = -1, ebo = -1;
    GLuint program = -1;
    GLint mvp_location = -1;
    std::array<unsigned int, 6> indices = {};
};

class TestBed : public Application {
public:
	TestBed(std::vector<std::string> args) : Application("AureLab Test Bed") {
		Log::Info("Hi from TestBed! Called with following CLI arguments: argc: {}, argv[0]: {}", args.size(), args[0]);

        Layer* layer = new Layer1();
        PushLayer(new Layer1());
	}
};

Application* CreateApplication(std::vector<std::string> args) {
	return new TestBed(args);
}