#include "FrameBuffer.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

#include <cassert>

FrameBuffer* FrameBuffer::Create(int width, int height) {
	FrameBuffer* fbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		fbo = new OpenGLFrameBuffer(width, height);
		break;
	default:
		assert(false); // Only OpenGL is implemented
	}
	return fbo;
}
