#include "GraphicsAPI.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLGraphicsAPI.h"

#include <cassert>

GraphicsAPI* GraphicsAPI::Create() {
	GraphicsAPI* graphicsAPI = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		graphicsAPI = new OpenGLGraphicsAPI();
		break;
	default:
		assert(false); // Only OpenGL is implemented
	}
	return graphicsAPI;
}
