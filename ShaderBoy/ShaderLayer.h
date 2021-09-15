#pragma once
#include "FileWatcher.h"

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/GraphicsAPI.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"

#include <glm/glm.hpp>
#include "imgui.h"

#include <memory>
#include <vector>
#include <filesystem>

struct Vertex {
    glm::vec2 position;
    glm::vec2 uv;
};

class ShaderLayer : public Layer {
public:
    ShaderLayer(std::filesystem::path filepath) 
        : Layer("Shader Layer"), filewatcher(filepath) {
        ga.reset(GraphicsAPI::Create());
        vao.reset(VertexArray::Create());
        shader.reset(Shader::Create(filepath.string()));
        filewatcher.Start();
    }

    virtual void OnAttach() override {
        std::vector<Vertex> vertices = {
            { {-0.9, -0.9}, {0.0, 0.0} },
            { { 0.9, -0.9}, {1.0, 0.0} },
            { {-0.9,  0.9}, {0.0, 1.0} },
            { { 0.9,  0.9}, {1.0, 1.0} },
        };
        vbo.reset(VertexBuffer::Create({
            VertexAttributeSpecification{ shader->GetAttribLocation("a_Position"), VertexAttributeSemantic::Position, VertexAttributeType::float32, 2, false},
            VertexAttributeSpecification{ shader->GetAttribLocation("a_UV"), VertexAttributeSemantic::UV, VertexAttributeType::float32, 2, false},
        }));
        vbo->SetVertices(vertices);

        std::vector<unsigned int> indices = {
            0, 1, 2,
            1, 3, 2,
        };
        IndexBuffer* ebo = IndexBuffer::Create();
        ebo->UploadIndices(indices);

        vao->AddVertexBuffer(*vbo);
        vao->SetIndexBuffer(*ebo);

        ga->SetClearColor({ 0.3, 0.2, 0.1, 1.0 });
    }

    virtual void OnUpdate(float ts) override {
        ga->Clear();

        shader->Bind();
        vao->Bind();
        ga->DrawIndexedTriangles(*vao, vao->GetIndexBuffer()->GetNumIndices());
    }

    virtual void OnDetach() override {}

    virtual void OnEvent(Event& ev) override {}

    virtual void OnImGuiRender() override {
        ImGui::Begin("ShaderBoy");
        ImGui::Text("Welcome to AureoLab ShaderBoy.\nUse generic parameters.");
        ImGui::End();
    }

private:
    std::unique_ptr<GraphicsAPI> ga = nullptr;
    std::unique_ptr<VertexArray> vao = nullptr;
    std::unique_ptr<VertexBuffer> vbo = nullptr;
    std::unique_ptr<Shader> shader = nullptr;
    FileWatcher filewatcher;
};