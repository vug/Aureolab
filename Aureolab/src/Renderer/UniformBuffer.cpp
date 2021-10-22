#include "UniformBuffer.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

#include <cassert>

int UniformBuffer::bindingPointCounter = 0;

UniformBuffer* UniformBuffer::Create(const std::string& name, unsigned int size) {
	UniformBuffer* ubo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		ubo = new OpenGLUniformBuffer(name, size);
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return ubo;
}