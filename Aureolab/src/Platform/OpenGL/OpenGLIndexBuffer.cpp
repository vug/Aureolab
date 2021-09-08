#include "OpenGLIndexBuffer.h"

#include <glad/glad.h>

#include <vector>

OpenGLIndexBuffer::OpenGLIndexBuffer() {
    glGenBuffers(1, &rendererID);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
	glDeleteBuffers(1, &rendererID);
}

void OpenGLIndexBuffer::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rendererID);
}

void OpenGLIndexBuffer::Unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGLIndexBuffer::UploadIndices(std::vector<unsigned int> indices) {
    Bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
}
