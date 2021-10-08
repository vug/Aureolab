#include "FrameBuffer.h"

#include "Core/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

#include <cassert>

FrameBuffer* FrameBuffer::Create(int width, int height, TextureFormat textureFormat) {
	FrameBuffer* fbo = nullptr;
	switch (GraphicsContext::graphicsAPI) {
	case GraphicsContext::API::OPENGL:
		fbo = new OpenGLFrameBuffer(width, height, textureFormat);
		break;
	default:
		assert(false); // Only OpenGL is implemented
	}
	return fbo;
}
