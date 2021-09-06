#include "VertexBuffer.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

#include <cassert>

VertexBuffer* VertexBuffer::Create() {
	VertexBuffer* vbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsAPI::OPENGL:
		//vbo = new OpenGLVertexBuffer();
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return vbo;
}
