#include "Shader.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <cassert>

Shader* Shader::Create(const std::string& filepath) {
	Shader* shader = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		shader = new OpenGLShader(filepath);
		break;
	default:
		assert(false); // Only OpenGL is implemented.
	}
	return shader;
}
