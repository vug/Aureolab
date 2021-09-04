#include "Window.h"
#include "GraphicsContext.h"
#include "Platform/Platform.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>

#include <cassert>

// Maybe in the future we can have a setter to change the graphics API on-the-fly
GraphicsAPI GraphicsContext::graphicsAPI = GraphicsAPI::OPENGL;

GraphicsContext* GraphicsContext::Create(Window* window) {
	GraphicsContext* gc = nullptr;
	switch (graphicsAPI) { 
	case GraphicsAPI::OPENGL:
		gc = new OpenGLContext(window);
		break;
	default:
		assert(false); // "Concrete graphics context for this API has not been implemented yet."
	} 
	return gc;
}
