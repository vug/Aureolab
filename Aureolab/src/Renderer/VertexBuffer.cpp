#include "VertexBuffer.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

VertexBuffer::~VertexBuffer() {
	delete vertices;
}

VertexBuffer* VertexBuffer::Create(std::vector<VertexAttributeSpecification> specs) {
	VertexBuffer* vbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		vbo = new OpenGLVertexBuffer(specs);
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return vbo;
}

const unsigned int VertexBuffer::GetNumVertices() const {
	return numVertices;
}
