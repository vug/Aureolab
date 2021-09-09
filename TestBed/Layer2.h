#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/GraphicsAPI.h"

#include <glad/glad.h>

#include <random>
#include <vector>

class Layer2 : public Layer {
public:
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	Layer2() : Layer("Point Sprites") { }

	void OnAttach() {
		std::seed_seq seed{ 123 };
		std::mt19937 mt(seed);
		std::uniform_real_distribution<float> uniform1(-1.0f, 1.0f);
		std::uniform_real_distribution<float> uniform2(0.0f, 1.0f);
		std::vector<Vertex> vertices = {};
		for (int i = 0; i < 500; i++) {
			vertices.push_back({ 
				{ uniform1(mt), uniform1(mt), uniform1(mt) }, 
				{ uniform2(mt), uniform2(mt), uniform2(mt) },
			});
		}

		shader = Shader::Create("assets/PointSprite.glsl");
		shader->Bind();

		VertexBuffer* vbo = VertexBuffer::Create({
			VertexAttributeSpecification{ shader->GetAttribLocation("a_Position"), VertexAttributeSemantic::Position, VertexAttributeType::float32, 3, false},
			VertexAttributeSpecification{ shader->GetAttribLocation("a_Color"), VertexAttributeSemantic::Color, VertexAttributeType::float32, 3, false},
		});
		vbo->SetVertices(vertices);

		vao = VertexArray::Create();
		vao->AddVertexBuffer(*vbo);

		ga = GraphicsAPI::Create();
		ga->Initialize();
		ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });

		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(16.0f); // default size
		
		// Order Independence
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	void OnUpdate(float ts) {
		glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
		glm::vec3 eye({ 0.0, 0.0, 2.0 });
		glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
		angle += ts * 0.5f;
		model = glm::rotate(model, ts, { 0, std::sin(angle), std::cos(angle) });

		glm::mat4 mvp = projection * view * model;

		ga->Clear();
		shader->Bind();
		shader->UploadUniformMat4("u_MVP", mvp);
		shader->UploadUniformFloat3("u_camPos", eye);
		ga->DrawArrayPoints(*vao);
	}

	void OnEvent(Event& ev) {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(Layer2::OnWindowResize));
	}

	void OnWindowResize(WindowResizeEvent& e) {
		Log::Debug("Layer2 received: {}", e.ToString());
		aspect = (float)e.GetWidth() / e.GetHeight();
	}

	void OnDetach() {
		delete shader;
		delete vao;
		delete ga;
	}
private:
	Shader* shader = nullptr;
	VertexArray* vao = nullptr;
	GraphicsAPI* ga = nullptr;
	float aspect = 1.0f;
	glm::mat4 model = glm::mat4(1.0f);
	float angle = 0.0f;
};