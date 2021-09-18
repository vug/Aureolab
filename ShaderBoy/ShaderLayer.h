#pragma once
#include "FileWatcher.h"

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"
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
        filewatcher.Start([&]() -> void {
            // Note that we don't recompile the shader here because this lambda function runs in filewatcher's thread
            // and that yhread does not have an OpenGL context, and shader compilation fails.
            shouldRecompileShader = true;
        });
    }

    virtual void OnAttach() override {
        std::vector<Vertex> vertices = {
            { {-1.0, -1.0}, {0.0, 0.0} },
            { { 1.0, -1.0}, {1.0, 0.0} },
            { {-1.0,  1.0}, {0.0, 1.0} },
            { { 1.0,  1.0}, {1.0, 1.0} },
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

        if (shouldRecompileShader) {
            shader->Recompile();
            shouldRecompileShader = false;
        }
        shader->Bind();
        shader->UploadUniformFloat("iTime", time);
        shader->UploadUniformFloat("iTimeDelta", ts);
        shader->UploadUniformFloat3("iResolution", viewportSize);
        shader->UploadUniformFloat4("iMouse", mouseState);
        vao->Bind();
        ga->DrawIndexedTriangles(*vao, (unsigned int)vao->GetIndexBuffer()->GetNumIndices());
        time += ts;
    }

    virtual void OnDetach() override {
        filewatcher.Stop(); // not needed really, the thread will be terminated while quitting the app
    }

    void OnResize(FrameBufferResizeEvent ev) {
        viewportSize.x = (float)ev.GetWidth();
        viewportSize.y = (float)ev.GetHeight();
    }

    void OnMouseMoved(MouseMovedEvent ev) {
        mouseState.x = ev.GetX();
        mouseState.y = viewportSize.y - ev.GetY(); // OpenGL and GLFW vertical coordinates are opposite
    }
    void OnMouseButtonPressed(MouseButtonPressedEvent ev) {
        if (ev.GetMouseButton() == 0) {
            mouseState.z = 1;
        }
        else if (ev.GetMouseButton() == 1) {
            mouseState.w = 1;
        }
    }
    void OnMouseButtonReleased(MouseButtonReleasedEvent ev) {
        if (ev.GetMouseButton() == 0) {
            mouseState.z = 0;
        }
        else if (ev.GetMouseButton() == 1) {
            mouseState.w = 0;
        }
    }

    virtual void OnEvent(Event& ev) override {
        auto dispatcher = EventDispatcher(ev);
        dispatcher.Dispatch<FrameBufferResizeEvent>(AL_BIND_EVENT_FN(ShaderLayer::OnResize));
        dispatcher.Dispatch<MouseMovedEvent>(AL_BIND_EVENT_FN(ShaderLayer::OnMouseMoved));
        dispatcher.Dispatch<MouseButtonPressedEvent>(AL_BIND_EVENT_FN(ShaderLayer::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(AL_BIND_EVENT_FN(ShaderLayer::OnMouseButtonReleased));
    }

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
    bool shouldRecompileShader = false;
    float time = 0.0f;
    glm::vec3 viewportSize = { 1000.0f, 1000.0f, 1.0f };
    glm::vec4 mouseState = { 0.0, 0.0, 0.0, 0.0 };
};