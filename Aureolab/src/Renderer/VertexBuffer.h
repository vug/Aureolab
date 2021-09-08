#pragma once

#include <any>
#include <cassert>
#include <vector>

enum class VertexAttributeSemantic {
	Position, 
	Color,
	Normal,
	UV,
};

enum class VertexAttributeType {
	int8,
	uint8,
	int16,
	uint16,
	int32,
	uint32,
	float32,
	float64,
};

class VertexSpecification {
public:
	unsigned int index;
	VertexAttributeSemantic semantic;
	VertexAttributeType type;
	int numComponents;
	bool normalized = false;
};

template <typename TVertex>
class VertexBuffer {
public:
	static VertexBuffer* Create(std::vector<VertexSpecification> specs, const std::vector<TVertex>& vertices = {});

	void SetVertices(const std::vector<TVertex>& newVertices);
	void AppendVertices(const std::vector<TVertex>& newVertices);
	void AppendVertex(const TVertex& vertex);
	void UpdateVertex(unsigned int index, const TVertex& vertex);
	void DeleteVertex(unsigned int index);

	virtual unsigned int GetVertexSize() = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;

protected:
	virtual void UploadBuffer(unsigned int size, void* data) = 0;
	std::vector<TVertex>* GetVertices();
private:
	void UploadVertices();
	void* vertices = new std::vector<TVertex>();
	void* data;
};


#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

template <typename TVertex>
VertexBuffer<TVertex>* VertexBuffer<TVertex>::Create(std::vector<VertexSpecification> specs, const std::vector<TVertex>& vertices) {
	VertexBuffer* vbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsAPI::OPENGL:
		vbo = new OpenGLVertexBuffer<TVertex>(specs);
		vbo->SetVertices(vertices);
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return vbo;
}

template<typename TVertex>
std::vector<TVertex>* VertexBuffer<TVertex>::GetVertices() {
	return (std::vector<TVertex>*)vertices;
}

template<typename TVertex>
void VertexBuffer<TVertex>::UploadVertices() {
	UploadBuffer(GetVertexSize() * GetVertices()->size(), GetVertices()->data());
}

template<typename TVertex>
void VertexBuffer<TVertex>::SetVertices(const std::vector<TVertex>& newVertices) {
	auto verts = GetVertices();
	*verts = newVertices; // copy operation
	UploadVertices();
}

template<typename TVertex>
void VertexBuffer<TVertex>::AppendVertex(const TVertex& vertex) {
	GetVertices()->push_back(vertex);
	UploadVertices();
}

template<typename TVertex>
void VertexBuffer<TVertex>::AppendVertices(const std::vector<TVertex>& newVertices) {
	auto verts = GetVertices();
	verts->insert(verts->end(), newVertices.begin(), newVertices.end());
	UploadVertices();
}

template<typename TVertex>
void VertexBuffer<TVertex>::UpdateVertex(unsigned int index, const TVertex& vertex) {
	assert(index < GetVertices()->size());
	(*GetVertices())[index] = vertex;
	UploadVertices();
}

template<typename TVertex>
void VertexBuffer<TVertex>::DeleteVertex(unsigned int index) {
	assert(index < vertices.size());
	vertices.erase(vertices.begin() + index);
	UploadVertices();
}