#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/GraphicsAPI.h"

#include "Renderer/Shader.h"
#include "Modeling/Modeling.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


class EditorLayer : public Layer {
public:
	EditorLayer() : Layer("Editor Layer") { }

	virtual void OnAttach() override {
		shader = Shader::Create("assets/shaders/BasicShader.glsl");
		std::vector<BasicVertex> vertices = LoadOBJ("assets/models/torus_smooth.obj");
		VertexBuffer* vbo = VertexBuffer::Create(BasicVertexAttributeSpecs);
		vbo->SetVertices(vertices);
		Log::Debug("num vertices: {}", vertices.size());
		vao = VertexArray::Create();
		vao->AddVertexBuffer(*vbo);

		ga = GraphicsAPI::Create();
		ga->Initialize();
		ga->Enable(GraphicsAbility::DepthTest);
		ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	}

	virtual void OnUpdate(float ts) override {
		static float time = 0.0f;
		static float angle = 0.0f;
		static glm::mat4 model = glm::mat4(1.0f);

		float red = std::sin(time);
		time += ts;

		float aspect = 1.0;
		glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
		glm::vec3 eye({ 0.0, 0.0, 5.0 });
		glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
		if (true) {
			angle += ts * 0.5f;
			model = glm::rotate(model, ts, { 0, std::sin(angle), std::cos(angle) });
		}
		glm::mat4 mv = view * model;
		glm::mat4 mvp = projection * mv;
		glm::mat4 normalMatrix = glm::inverse(mv);

		ga->SetClearColor({ red, 0.1f, 0.1f, 1.0f });
		ga->Clear();

		shader->Bind();
		shader->UploadUniformMat4("u_ModelViewPerspective", mvp); // needed for gl_Position;
		shader->UploadUniformInt("u_RenderType", 1); // Normal (2: UV)
		ga->DrawArrayTriangles(*vao);
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
	float aspect = 1.0f;

	Shader* shader = nullptr;
	VertexArray* vao = nullptr;
};