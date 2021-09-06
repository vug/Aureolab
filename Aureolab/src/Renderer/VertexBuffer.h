#pragma once

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

	virtual void SetVertices(const std::vector<TVertex>& newVertices) = 0;
	virtual void AppendVertices(const std::vector<TVertex>& newVertices) = 0;
	virtual void AppendVertex(const TVertex& vertex) = 0;
	virtual void UpdateVertex(unsigned int index, const TVertex& vertex) = 0;
	virtual void DeleteVertex(unsigned int index) = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
};


#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

template <typename TVertex>
VertexBuffer<TVertex>* VertexBuffer<TVertex>::Create(std::vector<VertexSpecification> specs, const std::vector<TVertex>& vertices) {
	VertexBuffer* vbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsAPI::OPENGL:
		vbo = new OpenGLVertexBuffer<TVertex>(specs, vertices);
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return vbo;
}