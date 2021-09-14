#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/GraphicsAPI.h"

#include "imgui.h"

#include <random>
#include <vector>

class Layer2 : public Layer {
public:
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	Layer2() : Layer("Point Sprites") { }

	const std::vector<Vertex>& CreatePoints(int numPoints) {
		std::seed_seq seed{ 123 };
		std::mt19937 mt(seed);
		std::uniform_real_distribution<float> uniform1(-1.0f, 1.0f);
		std::uniform_real_distribution<float> uniform2(0.0f, 1.0f);
		std::vector<Vertex>* vertices = new std::vector<Vertex>();
		for (int i = 0; i < numPoints; i++) {
			glm::vec3 pos = { uniform1(mt), uniform1(mt), uniform1(mt) };
			float distToCenter = glm::length(pos);
			glm::vec3 col = color1 * (1 - distToCenter) + color2 * distToCenter;
			vertices->push_back({ pos, col });
		}
		return *vertices;
	}

	virtual void OnAttach() override {
		shader = Shader::Create("assets/PointSprite.glsl");
		shader->Bind();

		vbo = VertexBuffer::Create({
			VertexAttributeSpecification{ shader->GetAttribLocation("a_Position"), VertexAttributeSemantic::Position, VertexAttributeType::float32, 3, false},
			VertexAttributeSpecification{ shader->GetAttribLocation("a_Color"), VertexAttributeSemantic::Color, VertexAttributeType::float32, 3, false},
		});
		auto points = CreatePoints(numPoints);
		vbo->SetVertices(points);

		vao = VertexArray::Create();
		vao->AddVertexBuffer(*vbo);

		ga = GraphicsAPI::Create();
		ga->Initialize();
		ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });

		ga->Enable(GraphicsAbility::PointSize); // Rasterized point diameter will be determined in shaders, gl_PointSize variable.
		//ga->SetDefaultPointSize(16.0f);
		
		// Order Independence
		ga->Disable(GraphicsAbility::DepthTest); // Every point primitive will be rendered, don't care about which is behind or front.
		ga->Enable(GraphicsAbility::Blend);
		ga->SetBlendingFunction(BlendingFactor::SourceAlpha, BlendingFactor::DestinationAlpha);
	}

	virtual void OnUpdate(float ts) override {
		glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
		glm::vec3 eye({ 0.0, 0.0, 4.0 });
		glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
		if (isModelSpinning) {
			angle += ts * 0.5f;
			//model = glm::rotate(model, ts, { 0, 1, 0 });
			model = glm::rotate(model, ts, { 0, std::sin(angle), std::cos(angle) });
		}

		glm::mat4 mvp = projection * view * model;

		ga->Clear();
		shader->Bind();
		shader->UploadUniformMat4("u_MVP", mvp);
		shader->UploadUniformFloat3("u_camPos", eye);
		shader->UploadUniformFloat("u_PointSize", pointSize);
		shader->UploadUniformFloat("u_FocalDistance", focalDistance);
		shader->UploadUniformFloat("u_BlurRadius", blurRadius);
		shader->UploadUniformFloat("u_DepthOfField", depthOfField);
		ga->DrawArrayPoints(*vao);
	}

	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(Layer2::OnWindowResize));
	}

	void OnWindowResize(WindowResizeEvent& e) {
		Log::Debug("Layer2 received: {}", e.ToString());
		aspect = (float)e.GetWidth() / e.GetHeight();
	}

	virtual void OnDetach() override {
		delete shader;
		delete vao;
		delete ga;
	}

	virtual void OnImGuiRender() override {
		ImGui::Begin("Point Sprites");
		ImGui::Text("with out of focus blurring");
		bool shouldRecreate = false;

		shouldRecreate|= ImGui::InputInt("Points", &numPoints, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SliderFloat("Size", &pointSize, 1.0f, 512.0f);
		ImGui::SliderFloat("Focal Dist", &focalDistance, 0.1f, 8.0f);
		ImGui::SliderFloat("Blur Radius", &blurRadius, 0.0f, 2.0f);
		ImGui::SliderFloat("DoF", &depthOfField, 0.0f, 4.0f);
		shouldRecreate |= ImGui::ColorEdit3("Color1", glm::value_ptr(color1));
		shouldRecreate |= ImGui::ColorEdit3("Color2", glm::value_ptr(color2));
		ImGui::Checkbox("Spin", &isModelSpinning);

		if (shouldRecreate) {
			Log::Info("New # of points: {}", numPoints);
			std::vector<Vertex> points = CreatePoints(numPoints);
			vbo->SetVertices(points);
		}
		ImGui::End();
	}

private:
	Shader* shader = nullptr;
	VertexArray* vao = nullptr;
	VertexBuffer* vbo = nullptr;
	GraphicsAPI* ga = nullptr;
	float aspect = 1.0f;
	glm::mat4 model = glm::mat4(1.0f);
	float angle = 0.0f;

	int numPoints = 1000;
	float pointSize = 64.0f;
	float focalDistance = 4.0f;
	float blurRadius = 0.5f;
	float depthOfField = 2.0f;
	glm::vec3 color2 = { 1.0, 0.0, 0.0 };
	glm::vec3 color1 = { 1.0, 1.0, 0.0 };
	bool isModelSpinning = true;
};