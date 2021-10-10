#include "Components.h"

#include "Core/Log.h"
#include "Renderer/VertexBuffer.h"
#include "Modeling/Modeling.h"

#include <array>

VertexArray* VertexArrayFromOBJ(const std::string& filepath) {
	std::vector<BasicVertex> vertices = LoadOBJ(filepath);
	VertexBuffer* vbo = VertexBuffer::Create(BasicVertexAttributeSpecs);
	vbo->SetVertices(vertices);
	Log::Debug("num vertices: {}", vertices.size());
	VertexArray* vao = VertexArray::Create();
	vao->AddVertexBuffer(*vbo);
	return vao;
}

MeshComponent::MeshComponent(const MeshComponent& other) : filepath(other.filepath) {
	LoadOBJ();
}

MeshComponent::MeshComponent(const std::string& filepath) : filepath(filepath) {
	LoadOBJ();
}

void MeshComponent::LoadOBJ() {
	vao = filepath.empty() ? nullptr : VertexArrayFromOBJ(filepath);
}



ProceduralMeshComponent::ProceduralMeshComponent() { GenerateMesh(); }

ProceduralMeshComponent::ProceduralMeshComponent(const ProceduralMeshComponent& other) : parameters(other.parameters) {
	GenerateMesh();
}

ProceduralMeshComponent::ProceduralMeshComponent(const Parameters& parameters) : parameters(parameters) {
	GenerateMesh();
}

void ProceduralMeshComponent::GenerateMesh() {
	VertexBuffer* vbo;
	if (vao == nullptr) {
		vao = VertexArray::Create();
		vbo = VertexBuffer::Create(BasicVertexAttributeSpecs);
		vao->AddVertexBuffer(*vbo);
	}
	else { vbo = vao->GetVertexBuffers()[0]; }

	std::vector<BasicVertex> vertices;
	switch (parameters.shape) {
		case Shape::Box:
			vertices = GenerateBox(parameters.box.dimensions);
			break;
		case Shape::Torus:
			// TODO:: implement Torus generation instead
			vertices = GenerateBox(parameters.box.dimensions);
			break;
	}
	vbo->SetVertices(vertices);
}