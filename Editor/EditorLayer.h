#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/GraphicsAPI.h"

#include "imgui.h"


class EditorLayer : public Layer {
public:
	EditorLayer() : Layer("Editor Layer") { }

	virtual void OnAttach() override {
		ga = GraphicsAPI::Create();
		ga->Initialize();
		ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	}

	virtual void OnUpdate(float ts) override {
		static float time = 0.0f;
		float red = std::sin(time);
		time += ts;
		ga->Clear();
		ga->SetClearColor({ red, 0.1f, 0.1f, 1.0f });
	}

	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(EditorLayer::OnWindowResize));
	}

	void OnWindowResize(WindowResizeEvent& e) {
		Log::Debug("Layer2 received: {}", e.ToString());
		aspect = (float)e.GetWidth() / e.GetHeight();
	}

	virtual void OnDetach() override {
		delete ga;
	}

	virtual void OnImGuiRender() override {
		ImGui::Begin("Editor Layer");
		ImGui::Text("Laylay...");
		ImGui::End();
	}

private:
	GraphicsAPI* ga = nullptr;
	float aspect;
};