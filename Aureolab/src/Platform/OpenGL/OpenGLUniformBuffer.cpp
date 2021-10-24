#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>

OpenGLUniformBuffer::OpenGLUniformBuffer(const std::string& name, unsigned int size) 
	: name(name), size(size) {
	bindingPoint = OpenGLUniformBuffer::bindingPointCounter++;

    glGenBuffers(1, &rendererID);
    glBindBuffer(GL_UNIFORM_BUFFER, rendererID);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
}

void OpenGLUniformBuffer::BlockBind(Shader* shader) {
    GLuint blockIndexForShader;
    blockIndexForShader = glGetUniformBlockIndex(shader->GetRendererID(), name.c_str());
    assert(blockIndexForShader != GL_INVALID_INDEX); // global uniform block is not used in this shader program
    glUniformBlockBinding(shader->GetRendererID(), blockIndexForShader, bindingPoint);
}

void OpenGLUniformBuffer::UploadData(const void* data) {
    Bind();
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void OpenGLUniformBuffer::Bind() const {
    glBindBuffer(GL_UNIFORM_BUFFER, rendererID);
}

void OpenGLUniformBuffer::Unbind() const {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
