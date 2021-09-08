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

class VertexBuffer {
public:
	template <typename TVertex>
	static VertexBuffer* Create(std::vector<VertexSpecification> specs, const std::vector<TVertex>& vertices = {});

	template <typename TVertex>
	void SetVertices(const std::vector<TVertex>& newVertices);
	template <typename TVertex>
	void AppendVertices(const std::vector<TVertex>& newVertices);
	template <typename TVertex>
	void AppendVertex(const TVertex& vertex);
	template <typename TVertex>
	void UpdateVertex(unsigned int index, const TVertex& vertex);
	template <typename TVertex>
	void DeleteVertex(unsigned int index);

	virtual unsigned int GetVertexSize() = 0;
	virtual void Bind() = 0;
	virtual void Unbind() = 0;

protected:
	virtual void UploadBuffer(size_t size, void* data) = 0;
	template <typename TVertex>
	std::vector<TVertex>* GetVertices();

private:
	template <typename TVertex>
	void UploadVertices();
	void* vertices = nullptr;
};


#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

template <typename TVertex>
VertexBuffer* VertexBuffer::Create(std::vector<VertexSpecification> specs, const std::vector<TVertex>& vertices) {
	VertexBuffer* vbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsAPI::OPENGL:
		vbo = new OpenGLVertexBuffer(specs);
		vbo->SetVertices(vertices);
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return vbo;
}

template<typename TVertex>
std::vector<TVertex>* VertexBuffer::GetVertices() {
	if (vertices == nullptr) {
		vertices = (void*)new std::vector<TVertex>();
	}
	return (std::vector<TVertex>*)vertices;
}

template <typename TVertex>
void VertexBuffer::UploadVertices() {
	UploadBuffer(GetVertexSize() * GetVertices<TVertex>()->size(), GetVertices<TVertex>()->data());
}

template<typename TVertex>
void VertexBuffer::SetVertices(const std::vector<TVertex>& newVertices) {
	auto verts = GetVertices<TVertex>();
	*verts = newVertices; // copy operation
	UploadVertices<TVertex>();
}

template<typename TVertex>
void VertexBuffer::AppendVertex(const TVertex& vertex) {
	GetVertices<TVertex>()->push_back(vertex);
	UploadVertices<TVertex>();
}

template<typename TVertex>
void VertexBuffer::AppendVertices(const std::vector<TVertex>& newVertices) {
	auto verts = GetVertices<TVertex>();
	verts->insert(verts->end(), newVertices.begin(), newVertices.end());
	UploadVertices<TVertex>();
}

template<typename TVertex>
void VertexBuffer::UpdateVertex(unsigned int index, const TVertex& vertex) {
	assert(index < GetVertices<TVertex>()->size());
	(*GetVertices<TVertex>())[index] = vertex;
	UploadVertices<TVertex>();
}

template <typename TVertex>
void VertexBuffer::DeleteVertex(unsigned int index) {
	assert(index < GetVertices()->size());
	GetVertices()->erase(GetVertices()->begin() + index);
	UploadVertices();
}