#include "OpenGLVertexBuffer.h"

#include "OpenGLVertexSpecification.h"

#include <glad/glad.h>

#include <cassert>
#include <numeric>

OpenGLVertexBuffer::OpenGLVertexBuffer(const std::vector<VertexAttributeSpecification>& specs)
	: attributeSpecs(specs) {
	glGenBuffers(1, &rendererID);
	Bind();

	auto attributeSizes = GetAttributeSizes();
	vertexSize = std::accumulate(attributeSizes.begin(), attributeSizes.end(), 0, [&](int sum, unsigned int size) {
		return sum + size; 
	});
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() {
	glDeleteBuffers(1, &rendererID);
}

// Size of all Vertex Attributes in Bytes, aka stride
const unsigned int OpenGLVertexBuffer::GetVertexSize() const {
	return vertexSize;
}

const std::vector<VertexAttributeSpecification>& OpenGLVertexBuffer::GetAttributeSpecs() const {
	return attributeSpecs;
}

const std::vector<unsigned int> OpenGLVertexBuffer::GetAttributeSizes() const {
	std::vector<unsigned int> sizes = {};
	for (auto& spec : attributeSpecs) {
		unsigned int componentSize = TypeSize(spec.type);
		unsigned int size = componentSize * spec.numComponents;
		sizes.push_back(size);
	}
	return sizes;
}

void OpenGLVertexBuffer::UploadBuffer(size_t size, void* data) {
	Bind();
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)size, data, GL_STATIC_DRAW);
}

void OpenGLVertexBuffer::Bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, rendererID);
}

void OpenGLVertexBuffer::Unbind() const {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}