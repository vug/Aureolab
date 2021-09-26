#include "GraphicsContext.h"

#include "Window.h"
#include "Platform/Platform.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>

#include <cassert>

GraphicsContext* GraphicsContext::instance = nullptr;
Window* GraphicsContext::window = nullptr;

void GraphicsContext::Initialize(Window* window) {
	GraphicsContext::window = window;
	GraphicsContext::Get(); // To actually create a GraphicsContext before calling any GraphicsAPI functions.
}

// Maybe in the future we can have a setter to change the graphics API on-the-fly
GraphicsContext::API GraphicsContext::graphicsAPI = API::OPENGL;

GraphicsContext* GraphicsContext::Get() {
	if (instance == nullptr) {
		switch (PlatformUtils::GetPlatform()) {
		case Platform::WINDOWS:
			instance = new OpenGLContext(window);
			break;
		default:
			assert(false); // Only implemented for Windows
		}
	}
	return instance;
}
