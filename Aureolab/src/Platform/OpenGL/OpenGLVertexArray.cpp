#include "OpenGLVertexArray.h"

#include "OpenGLVertexSpecification.h"

#include <glad/glad.h>


OpenGLVertexArray::OpenGLVertexArray() {
    glGenVertexArrays(1, &rendererID);
    Bind();
}

OpenGLVertexArray::~OpenGLVertexArray() {
	glDeleteVertexArrays(1, &rendererID);
}

void OpenGLVertexArray::Bind() const {
    glBindVertexArray(rendererID);
}

void OpenGLVertexArray::Unbind() const {
    glBindVertexArray(0);
}

void OpenGLVertexArray::AddVertexBuffer(const VertexBuffer& vertexBuffer) {
    Bind();

	auto specs = vertexBuffer.GetAttributeSpecs();
	auto sizes = vertexBuffer.GetAttributeSizes();
	auto stride = vertexBuffer.GetVertexSize();
	unsigned int offset = 0;
	for (unsigned int ix = 0; ix < specs.size(); ix++) {
		const auto& spec = specs[ix];
		glVertexAttribPointer(spec.index, spec.numComponents, ALTypeToGLType(spec.type), spec.normalized, stride, (void*)(std::uintptr_t)offset);
		glEnableVertexAttribArray(spec.index);
		offset = sizes[ix];
	}
	vertexBuffers.push_back(&vertexBuffer);
}

void OpenGLVertexArray::SetIndexBuffer(const IndexBuffer& indexBuffer) {
	Bind();
	indexBuffer.Bind();
	this->indexBuffer = &indexBuffer;
}