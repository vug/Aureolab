#include "VertexBuffer.h"

#include "Platform/OpenGL/OpenGLVertexBuffer.h"

VertexBuffer* VertexBuffer::Create(std::vector<VertexSpecification> specs) {
	VertexBuffer* vbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsAPI::OPENGL:
		vbo = new OpenGLVertexBuffer(specs);
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return vbo;
}