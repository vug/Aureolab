#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Renderer/GraphicsAPI.h"

#include <glad/glad.h>
//#define GLM_FORCE_CTOR_INIT /* initialize vectors and matrices */
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <iostream>

class Layer1 : public Layer {
public:
    Layer1() : Layer("FirstLayer") { }

    virtual void OnAttach() override {
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

        shader = Shader::Create("assets/shaders/VertexColor2D.glsl");

        std::vector<VertexAttributeSpecification> specs = {
            VertexAttributeSpecification{ shader->GetAttribLocation("vPos"), VertexAttributeSemantic::Position, VertexAttributeType::float32, 2, false},
            VertexAttributeSpecification{ shader->GetAttribLocation("vCol"), VertexAttributeSemantic::Color, VertexAttributeType::float32, 3, false},
        };
        vb = VertexBuffer::Create(specs);
        vb->SetVertices(vertices1);
        vb->AppendVertices(vertices2);
        vb->AppendVertex(vertex3);
        vb->UpdateVertex(3, vertex4);

        ib = IndexBuffer::Create();
        ib->UploadIndices(indices);

        va = VertexArray::Create();
        va->AddVertexBuffer(*vb);
        va->SetIndexBuffer(*ib);

        GraphicsAPI::Get()->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
    }

    virtual void OnUpdate(float ts) override {
        mvp *= glm::rotate(glm::mat4(1.0f), ts, glm::vec3(0.0f, 0.0f, 1.0f));

        GraphicsAPI::Get()->Clear();
        shader->Bind();
        shader->UploadUniformMat4("MVP", mvp);
        GraphicsAPI::Get()->DrawIndexedTriangles(*va);
    }

    virtual void OnDetach() override {
        // optional
        delete vb;
        delete ib;
        delete va;
        delete shader;
    }

    virtual void OnEvent(Event& ev) override {
        auto dispatcher = EventDispatcher(ev);
        dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(Layer1::OnWindowResize));
        dispatcher.Dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(Layer1::OnKeyPressed));
        dispatcher.Dispatch<KeyReleasedEvent>(AL_BIND_EVENT_FN(Layer1::OnKeyReleased));
        dispatcher.Dispatch<MouseMovedEvent>(AL_BIND_EVENT_FN(Layer1::OnMouseMoved));
        dispatcher.Dispatch<MouseScrolledEvent>(AL_BIND_EVENT_FN(Layer1::OnMouseScrolled));
        dispatcher.Dispatch<MouseButtonPressedEvent>(AL_BIND_EVENT_FN(Layer1::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(AL_BIND_EVENT_FN(Layer1::OnMouseButtonReleased));
    }

    virtual void OnImGuiRender() override {}

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
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    VertexArray* va = nullptr;
    std::vector<unsigned int> indices = {};
    glm::mat4 mvp = glm::mat4(1.0f);
    Shader* shader = nullptr;
};