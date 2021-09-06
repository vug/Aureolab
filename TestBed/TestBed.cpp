#include "Core/Log.h"
#include "Core/EntryPoint.h"
#include "Core/Application.h"
#include "Core/Layer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/VertexBuffer.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

#include <glad/glad.h>
//#define GLM_FORCE_CTOR_INIT /* initialize vectors and matrices */
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <iostream>

class Layer1 : public Layer {
public:
    Layer1() : Layer("FirstLayer") { }

    void OnAttach() {
		struct Vertex {
			glm::vec2 pos;
			glm::vec3 color;
		};

		std::vector<Vertex> vertices1 = {
			{ {-0.6f, -0.4f}, {1.f, 0.f, 0.f} },
			{  {0.6f, -0.4f}, {0.f, 1.f, 0.f} },
		};
		std::vector<Vertex> vertices2 = {
		   {  {0.f,  0.6f}, {0.f, 0.f, 1.f} },
		};
		Vertex vertex3 = { {-0.3f,  0.8f}, {1.f, 0.f, 1.f} };
        Vertex vertex4 = { {-0.3f,  0.8f}, {1.f, 1.f, 0.f} };

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

        std::vector<VertexSpecification> specs = {
            VertexSpecification{ (unsigned int)vpos_location, VertexAttributeSemantic::Position, VertexAttributeType::float32, 2, false },
            VertexSpecification{ (unsigned int)vcol_location, VertexAttributeSemantic::Color, VertexAttributeType::float32, 3, false },
        };
        auto vb = new OpenGLVertexBuffer<Vertex>(specs, vertices1);
        vb->AppendVertices(vertices2);
        vb->AppendVertex(vertex3);
        vb->UpdateVertex(3, vertex4);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);
    }

    void OnUpdate(float ts) {
        mvp *= glm::rotate(glm::mat4(1.0f), ts, glm::vec3(0.0f, 0.0f, 1.0f));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
    }

    void OnDetach() {
        // optional
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteProgram(program);
    }

    void OnEvent(Event& ev) {
        auto dispatcher = EventDispatcher(ev);
        dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(Layer1::OnWindowResize));
        dispatcher.Dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(Layer1::OnKeyPressed));
        dispatcher.Dispatch<KeyReleasedEvent>(AL_BIND_EVENT_FN(Layer1::OnKeyReleased));
        dispatcher.Dispatch<MouseMovedEvent>(AL_BIND_EVENT_FN(Layer1::OnMouseMoved));
        dispatcher.Dispatch<MouseScrolledEvent>(AL_BIND_EVENT_FN(Layer1::OnMouseScrolled));
        dispatcher.Dispatch<MouseButtonPressedEvent>(AL_BIND_EVENT_FN(Layer1::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(AL_BIND_EVENT_FN(Layer1::OnMouseButtonReleased));
    }

    void OnWindowResize(WindowResizeEvent& e) {
        Log::Debug("Layer1 received: {}", e.ToString());
    }

    void OnKeyPressed(KeyPressedEvent& ev) {
        Log::Debug("Layer1 received: {}", ev.ToString());
    }

    void OnKeyReleased(KeyReleasedEvent& ev) {
        Log::Debug("Layer1 received: {}", ev.ToString());
    }

    void OnMouseMoved(MouseMovedEvent& ev) {
        Log::Debug("Layer1 received: {}", ev.ToString());
    }

    void OnMouseScrolled(MouseScrolledEvent& ev) {
        Log::Debug("Layer1 received: {}", ev.ToString());
    }

    void OnMouseButtonPressed(MouseButtonPressedEvent& ev) {
        Log::Debug("Layer1 received: {}", ev.ToString());
    }

    void OnMouseButtonReleased(MouseButtonReleasedEvent& ev) {
        Log::Debug("Layer1 received: {}", ev.ToString());
    }

private:
    GLuint vbo = -1, vao = -1, ebo = -1;
    GLuint program = -1;
    GLint mvp_location = -1;
    std::array<unsigned int, 6> indices = {};
    glm::mat4 mvp = glm::mat4(1.0f);
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