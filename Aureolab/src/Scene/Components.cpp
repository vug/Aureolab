#include "Components.h"

#include "Core/Log.h"
#include "Renderer/VertexBuffer.h"
#include "Modeling/Modeling.h"

VertexArray* VertexArrayFromOBJ(const std::string& filepath) {
	std::vector<BasicVertex> vertices = LoadOBJ(filepath);
	VertexBuffer* vbo = VertexBuffer::Create(BasicVertexAttributeSpecs);
	vbo->SetVertices(vertices);
	Log::Debug("num vertices: {}", vertices.size());
	VertexArray* vao = VertexArray::Create();
	vao->AddVertexBuffer(*vbo);
	return vao;
}

MeshComponent::MeshComponent(const MeshComponent& other) {
	vao = VertexArrayFromOBJ(other.filepath);
}

MeshComponent::MeshComponent(const std::string& filepath) {
	vao = VertexArrayFromOBJ(filepath);
}