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

// Same as VertexSpecification, additionaly OpenGL version has a size field in bytes
OpenGLVertexSpecification::OpenGLVertexSpecification(const VertexSpecification& spec)
	: VertexSpecification(spec) {
	unsigned int componentSize = TypeSize(spec.type);
	size = componentSize * spec.numComponents;
}




OpenGLVertexBuffer::OpenGLVertexBuffer(std::vector<VertexSpecification> specs) { // copies vertices data
	// Generate Buffer
	glGenBuffers(1, &rendererID);
	Bind();

	// Prepare Attributes
	for (auto& spec : specs) {
		attributeSpecs.push_back(spec);
	}
	vertexSize = std::accumulate(attributeSpecs.begin(), attributeSpecs.end(), 0, [&](int sum, const OpenGLVertexSpecification& curr) { return sum + curr.size; });
	assert(vertexSize > 0); // needs vertexSize computed
	for (unsigned int ix = 0; ix < attributeSpecs.size(); ix++) {
		const auto& spec = attributeSpecs[ix];
		unsigned int offset = ix == 0 ? 0 : attributeSpecs[ix - 1].size;
		glVertexAttribPointer(spec.index, spec.numComponents, ALTypeToGLType(spec.type), spec.normalized, vertexSize, (void*)(std::uintptr_t)offset);
		glEnableVertexAttribArray(spec.index);
	}
}

unsigned int OpenGLVertexBuffer::GetVertexSize() {
	return vertexSize;
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