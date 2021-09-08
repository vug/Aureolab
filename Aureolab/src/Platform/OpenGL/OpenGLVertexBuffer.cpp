#include "OpenGLVertexBuffer.h"

#include <glad/glad.h>

#include <cassert>
#include <numeric>

// Helper to convert AureoLab data type to OpenGL data type
GLenum ALTypeToGLType(VertexAttributeType alType) {
	switch (alType) {
	case VertexAttributeType::int8:
		return GL_BYTE;
	case VertexAttributeType::uint8:
		return GL_UNSIGNED_BYTE;
	case VertexAttributeType::int16:
		return GL_SHORT;
	case VertexAttributeType::uint16:
		return GL_UNSIGNED_SHORT;
	case VertexAttributeType::int32:
		return GL_INT;
	case VertexAttributeType::uint32:
		return GL_UNSIGNED_INT;
	case VertexAttributeType::float32:
		return GL_FLOAT;
	case VertexAttributeType::float64:
		return GL_DOUBLE;
	default:
		assert(false); // AL type not implemented
		return -1;
	}
}

// Helper to get OpenGL type size of AureoLab data type
inline unsigned int TypeSize(VertexAttributeType type) {
	switch (type) {
	case VertexAttributeType::int8:
		return sizeof(GL_BYTE);
	case VertexAttributeType::uint8:
		return sizeof(GL_UNSIGNED_BYTE);
	case VertexAttributeType::int16:
		return sizeof(GL_SHORT);
	case VertexAttributeType::uint16:
		return sizeof(GL_UNSIGNED_SHORT);
	case VertexAttributeType::int32:
		return sizeof(GL_INT);
	case VertexAttributeType::uint32:
		return sizeof(GL_UNSIGNED_INT);
	case VertexAttributeType::float32:
		return sizeof(GL_FLOAT);
	case VertexAttributeType::float64:
		return sizeof(GL_DOUBLE);
	default:
		assert(false); // AL type not implemented
		return -1;
	}
}

OpenGLVertexBuffer::OpenGLVertexBuffer(const std::vector<VertexAttributeSpecification>& specs)
	: attributeSpecs(specs) {
	glGenBuffers(1, &rendererID);
	Bind();

	auto attributeSizes = GetAttributeSizes();
	vertexSize = std::accumulate(attributeSizes.begin(), attributeSizes.end(), 0, [&](int sum, unsigned int size) {
		return sum + size; 
	});

	unsigned int offset = 0;
	for (unsigned int ix = 0; ix < attributeSpecs.size(); ix++) {
		const auto& spec = attributeSpecs[ix];
		glVertexAttribPointer(spec.index, spec.numComponents, ALTypeToGLType(spec.type), spec.normalized, vertexSize, (void*)(std::uintptr_t)offset);
		glEnableVertexAttribArray(spec.index);
		offset = attributeSizes[ix];
	}
}

// Size of all Vertex Attributes in Bytes, aka stride
unsigned int OpenGLVertexBuffer::GetVertexSize() {
	return vertexSize;
}

std::vector<unsigned int> OpenGLVertexBuffer::GetAttributeSizes() {
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

void OpenGLVertexBuffer::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, rendererID);
}

void OpenGLVertexBuffer::Unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}