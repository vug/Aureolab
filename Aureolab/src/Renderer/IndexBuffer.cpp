#include "IndexBuffer.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLIndexBuffer.h"

#include <cassert>

IndexBuffer* IndexBuffer::Create() {
	IndexBuffer* ebo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		ebo = new OpenGLIndexBuffer();
		break;
	default:
		assert(false); // Only OpenGL is implemented
	}
	return ebo;
}
