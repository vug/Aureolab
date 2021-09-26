#include "OpenGLFrameBuffer.h"

#include "Core/Log.h"

#include <glad/glad.h>

#include <cassert>

OpenGLFrameBuffer::OpenGLFrameBuffer(int width, int height) {
	glGenFramebuffers(1, &rendererID);

	Bind();
	// generate color buffer texture
	unsigned int colorRendererID;
	glGenTextures(1, &colorRendererID);
	glBindTexture(GL_TEXTURE_2D, colorRendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindTexture(GL_TEXTURE_2D, 0);
	colorRendererIDs.push_back(colorRendererID);

	// generate depth buffer texture
	glGenTextures(1, &depthRendererID);
	glBindTexture(GL_TEXTURE_2D, depthRendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach them to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorRendererID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRendererID, 0);

	// Check FBO completion
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		Log::Debug("Framebuffer complete.");
	}
	else {
		Log::Warning("Framebuffer incomplete.");
	}
	Unbind();
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

unsigned int OpenGLFrameBuffer::GetColorAttachmentRendererID(unsigned int index) const {
	assert(index < colorRendererIDs.size()); // attachment with that index does not exist.
	return colorRendererIDs[index];
}

void OpenGLFrameBuffer::Resize(int width, int height) {
	Bind();
	for (unsigned int id : colorRendererIDs) {
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindTexture(GL_TEXTURE_2D, depthRendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	Unbind();
}
