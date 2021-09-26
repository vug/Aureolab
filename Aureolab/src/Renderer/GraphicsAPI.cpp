#include "GraphicsAPI.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLGraphicsAPI.h"

#include <cassert>

GraphicsAPI* GraphicsAPI::instance = nullptr;

GraphicsAPI* GraphicsAPI::Get() {
	if (instance == nullptr) {
		switch (GraphicsContext::graphicsAPI) {
		case GraphicsContext::API::OPENGL:
			instance = new OpenGLGraphicsAPI();
			break;
		default:
			assert(false); // Only implemented for OpenGL
		}
	}
	return instance;
}
