#include "OpenGLFrameBuffer.h"

#include "Core/Log.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>

OpenGLFrameBuffer::OpenGLFrameBuffer(int width, int height, TextureFormat textureFormat) 
		: textureFormat(textureFormat) {
	glGenFramebuffers(1, &rendererID);

	Bind();
	// generate color buffer texture
	unsigned int colorRendererID;
	glGenTextures(1, &colorRendererID);
	glBindTexture(GL_TEXTURE_2D, colorRendererID);
	switch (textureFormat) {
	case TextureFormat::RGBA8:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		break;
	case TextureFormat::RED_INTEGER:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, width, height, 0, GL_RED_INTEGER, GL_INT, 0);
		break;
	default:
		assert(false); // unknown TextureFormat
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindTexture(GL_TEXTURE_2D, 0);
	colorRendererIDs.push_back(colorRendererID);

	// generate depth & stencil buffer texture
	glGenTextures(1, &depthRendererID);
	glBindTexture(GL_TEXTURE_2D, depthRendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach them to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorRendererID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthRendererID, 0);

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
		switch (textureFormat) {
		case TextureFormat::RGBA8:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			break;
		case TextureFormat::RED_INTEGER:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, width, height, 0, GL_RED_INTEGER, GL_INT, 0);
			break;
		}
		
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindTexture(GL_TEXTURE_2D, depthRendererID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	Unbind();
}

void OpenGLFrameBuffer::Clear(int clearValue, unsigned int index) {
	glClearTexImage(GetColorAttachmentRendererID(index), 0, GL_RED_INTEGER, GL_INT, &clearValue);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLFrameBuffer::Clear(glm::vec4 clearColor, unsigned int index) {
	glClearTexImage(GetColorAttachmentRendererID(index), 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(clearColor));
	glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLFrameBuffer::ReadPixel(int& pixel, int x, int y, unsigned int index) {
	glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel);
}

void OpenGLFrameBuffer::ReadPixel(glm::vec4& pixel, int x, int y, unsigned int index) {
	glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
}
