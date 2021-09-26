#include "OpenGLFrameBuffer.h"

#include <glad/glad.h>

OpenGLFrameBuffer::OpenGLFrameBuffer() {
	glGenFramebuffers(1, &rendererID);
}

OpenGLFrameBuffer::~OpenGLFrameBuffer() {
	glDeleteFramebuffers(1, &rendererID);
}

void OpenGLFrameBuffer::Bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, rendererID);
}

void OpenGLFrameBuffer::Unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
