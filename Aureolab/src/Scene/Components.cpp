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

std::vector<BasicVertex> GenerateBox(glm::vec3 dimensions) {
	glm::vec3 halfDim = dimensions * 0.5f;
	float width = halfDim.x, height = halfDim.y, depth = halfDim.z;

	// corners
	glm::vec3 p000 = { -width, -height, -depth };
	glm::vec3 p001 = { -width, -height, +depth };
	glm::vec3 p010 = { -width, +height, -depth };
	glm::vec3 p011 = { -width, +height, +depth };
	glm::vec3 p100 = { +width, -height, -depth };
	glm::vec3 p101 = { +width, -height, +depth };
	glm::vec3 p110 = { +width, +height, -depth };
	glm::vec3 p111 = { +width, +height, +depth };

	// normals
	glm::vec3 nFront = { 0.0f, 0.0f, 1.0f };
	glm::vec3 nBack = -nFront;
	glm::vec3 nLeft = { 1.0f, 0.0, 0.0 };
	glm::vec3 nRight = -nLeft;
	glm::vec3 nUp = { 0.0f, 1.0f, 0.0f }; 
	glm::vec3 nDown = -nUp;

	// faces (four corners in CCW, 1 normal)
	struct Face { std::array<glm::vec3, 4> corners; glm::vec3 normal; };
	Face fBack = { { p010, p110, p100, p000, }, nBack };
	Face fFront = { { p001, p101, p111, p011, }, nFront };
	Face fLeft = { { p110, p111, p101, p100, }, nLeft };
	Face fRight = { { p000, p001, p011, p010, }, nRight };
	Face fUp = { { p010, p011, p111, p110 }, nUp };
	Face fDown = { { p100, p101, p001, p000 }, nDown };

	std::vector<BasicVertex> vertices;
	int indices[] = { 0, 1, 2,    // triangle 1 of quad
					  0, 2, 3, }; // triangle 2 of quad
	glm::vec2 uvs[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
	for (auto& face : { fBack, fFront, fLeft, fRight, fUp, fDown }) {
		for (int ix : indices) {
			vertices.emplace_back<BasicVertex>(
				{ face.corners[ix], face.normal, uvs[ix], {},}
			);
		}
	}
	return vertices;
};

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