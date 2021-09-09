#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/Shader.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/GraphicsAPI.h"

#include <glad/glad.h>

#include <vector>

class Layer2 : public Layer {
public:
	struct Vertex {
		glm::vec3 position;
	};

	Layer2() : Layer("Point Sprites") { }

	void OnAttach() {
		std::vector<Vertex> vertices = { 
			{{ 0.0f,  0.0f,  0.0f}},
			{{ 0.1f,  0.2f, -0.5f}},
			{{-0.5f,  0.4f,  0.5f}},
		};
		std::vector<unsigned int> indices = {0, 1, 2};

		shader = Shader::Create("assets/PointSprite.glsl");

		VertexBuffer* vbo = VertexBuffer::Create({
			VertexAttributeSpecification{ shader->GetAttribLocation("a_Position"), VertexAttributeSemantic::Position, VertexAttributeType::float32, 3, false},
		});
		vbo->SetVertices(vertices);

		IndexBuffer* ebo = IndexBuffer::Create();
		ebo->UploadIndices(indices);

		vao = VertexArray::Create();
		vao->AddVertexBuffer(*vbo);
		vao->SetIndexBuffer(*ebo);

		ga = GraphicsAPI::Create();
		ga->Initialize();
		ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });

		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(16.0f); // default size
		
		// Order Independence
		//glDisable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE);
	}

	void OnUpdate(float ts) {
		glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
		glm::vec3 eye({ 0.0, 0.0, 2.0 });
		glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
		glm::mat4 model = glm::mat4(1.0f);

		glm::mat4 mvp = projection * view * model;

		ga->Clear();
		shader->Bind();
		shader->UploadUniformMat4("u_MVP", mvp);
		shader->UploadUniformFloat3("u_camPos", eye);
		ga->DrawIndexedPoints(*vao);
	}

	void OnEvent(Event& ev) {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(Layer2::OnWindowResize));
	}

	void OnWindowResize(WindowResizeEvent& e) {
		Log::Debug("Layer1 received: {}", e.ToString());
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
};