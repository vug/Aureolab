#pragma once

#include "Core/Layer.h"
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
		//Vertex() = default;
	};

	Layer2() : Layer("Point Sprites") { }

	void OnAttach() {
		std::vector<Vertex> vertices = { 
			{{0.1f, 0.2f, 0.85f}},
			{{0.2f, 0.3f, 0.25f}},
			{{0.3f, 0.4f, 0.55f}},
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
		ga->SetClearColor({ 0.9f, 0.8f, 0.7f, 1.0f });
		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(16.0f); // default size
		//glEnable(GL_CULL_FACE);
		//glDisable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE);
	}

	void OnUpdate(float ts) {
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		//projection = glm::perspective(50., 1.0, 0.01, 100.0);

		ga->Clear();
		shader->Bind();
		shader->UploadUniformMat4("u_ModelMatrix", model);
		shader->UploadUniformMat4("u_ProjectionMatrix", projection);
		ga->DrawIndexedPoints(*vao);
	}

	void OnEvent(Event& ev) {

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
};