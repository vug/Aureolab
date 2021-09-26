#include "FrameBuffer.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

#include <cassert>

FrameBuffer* FrameBuffer::Create() {
	FrameBuffer* fbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		fbo = new OpenGLFrameBuffer();
		break;
	default:
		assert(false); // Only OpenGL is implemented
	}
	return fbo;
}
