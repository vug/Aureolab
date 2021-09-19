#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/GraphicsAPI.h"
#include "Modeling/Modeling.h"

#include <glad/glad.h>
//#define GLM_FORCE_CTOR_INIT /* initialize vectors and matrices */
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>

#include <array>
#include <iostream>

class Layer3 : public Layer {
public:
    Layer3() : Layer("OBJ Loading") { }

    virtual void OnAttach() override {
        std::vector<BasicVertex> vertices = LoadOBJ("assets/torus.obj");
        Log::Info("num vertices: {}", vertices.size());

        shader = Shader::Create("assets/BasicShader.glsl");

        Log::Info("attribute locations: {} {} {} {}", shader->GetAttribLocation("a_Position"), shader->GetAttribLocation("a_Normal"),
            shader->GetAttribLocation("a_TexCoord"), shader->GetAttribLocation("a_Color"));

        std::vector<VertexAttributeSpecification> specs = {
            VertexAttributeSpecification{ shader->GetAttribLocation("a_Position"), VertexAttributeSemantic::Position, VertexAttributeType::float32, 3, false},
            VertexAttributeSpecification{ shader->GetAttribLocation("a_Normal"), VertexAttributeSemantic::Normal, VertexAttributeType::float32, 3, false},
            VertexAttributeSpecification{ shader->GetAttribLocation("a_TexCoord"), VertexAttributeSemantic::UV, VertexAttributeType::float32, 2, false},
            VertexAttributeSpecification{ shader->GetAttribLocation("a_Color"), VertexAttributeSemantic::Color, VertexAttributeType::float32, 4, false},
        };

        vb = VertexBuffer::Create(specs);
        vb->SetVertices(vertices);

        va = VertexArray::Create();
        va->AddVertexBuffer(*vb);

        ga = GraphicsAPI::Create();
        ga->Initialize();
        ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        ga->Enable(GraphicsAbility::DepthTest);
        ga->Enable(GraphicsAbility::FaceCulling);
        ga->Disable(GraphicsAbility::Blend);
    }

    virtual void OnUpdate(float ts) override {
        float aspect = 1.0;
        bool isModelSpinning = true;
        glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
        glm::vec3 eye({ 0.0, 0.0, 5.0 });
        glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
        if (isModelSpinning) {
            angle += ts * 0.5f;
            model = glm::rotate(model, ts, { 0, std::sin(angle), std::cos(angle) });
        }

        glm::mat4 mvp = projection * view * model;

        ga->Clear();
        shader->Bind();
        shader->UploadUniformMat4("MVP", mvp);
        ga->DrawArrayTriangles(*va);
    }

    virtual void OnDetach() override {
        // optional
        delete vb;
        delete ib;
        delete va;
        delete shader;
        delete ga;
    }

    virtual void OnEvent(Event& ev) override {
        auto dispatcher = EventDispatcher(ev);
    }

    virtual void OnImGuiRender() override {}

private:
    GraphicsAPI* ga = nullptr;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    VertexArray* va = nullptr;
    std::vector<unsigned int> indices = {};
    glm::mat4 model = glm::mat4(1.0f);
    Shader* shader = nullptr;
    float angle = 0.0f;
};