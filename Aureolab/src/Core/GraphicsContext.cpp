#include "Window.h"
#include "GraphicsContext.h"
#include "Platform/Platform.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>

#include <cassert>

// Maybe in the future we can have a setter to change the graphics API on-the-fly
GraphicsContext::API GraphicsContext::graphicsAPI = API::OPENGL;

GraphicsContext* GraphicsContext::Create(Window* window) {
	GraphicsContext* gc = nullptr;
	switch (graphicsAPI) { 
	case API::OPENGL:
		gc = new OpenGLContext(window);
		break;
	default:
		assert(false); // "Concrete graphics context for this API has not been implemented yet."
	} 
	return gc;
}
