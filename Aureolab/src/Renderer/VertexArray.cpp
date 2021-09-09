#include "VertexArray.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

VertexArray* VertexArray::Create() {
	VertexArray* vao = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		vao = new OpenGLVertexArray();
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return vao;
}
