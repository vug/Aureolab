#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/GraphicsAPI.h"

#include "imgui.h"

class ShaderLayer : public Layer {
public:
    ShaderLayer() : Layer("Shader Layer") {}

    virtual void OnAttach() override {
        ga = GraphicsAPI::Create();
        ga->SetClearColor({ 0.3, 0.2, 0.1, 1.0 });
    }

    virtual void OnUpdate(float ts) override {
        ga->Clear();
    }

    virtual void OnDetach() override {}

    virtual void OnEvent(Event& ev) override {}

    virtual void OnImGuiRender() override {
        ImGui::Begin("ShaderBoy");
        ImGui::Text("Welcome to AureoLab ShaderBoy.\nUse generic parameters.");
        ImGui::End();
    }

private:
    GraphicsAPI* ga = nullptr;
};