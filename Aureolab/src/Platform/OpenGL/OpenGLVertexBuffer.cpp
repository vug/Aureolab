#include "OpenGLVertexBuffer.h"

#include "Core/Log.h"

#include <glad/glad.h>

#include <numeric>

// Helper to convert AureoLab data type to OpenGL data type
inline GLenum ALTypeToGLType(VertexAttributeType alType) {
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
	}
}

// Same as VertexSpecification, additionaly OpenGL version has a size field in bytes
OpenGLVertexSpecification::OpenGLVertexSpecification(const VertexSpecification& spec)
	: VertexSpecification(spec) {
	unsigned int componentSize = TypeSize(spec.type);
	size = componentSize * spec.numComponents;
}

OpenGLVertexBuffer::OpenGLVertexBuffer() {
	glGenBuffers(1, &rendererID);
	glBindBuffer(GL_ARRAY_BUFFER, rendererID);
}

OpenGLVertexBuffer::OpenGLVertexBuffer(std::vector<VertexSpecification> specs, unsigned int numVertices, void* data) {
	glGenBuffers(1, &rendererID);
	glBindBuffer(GL_ARRAY_BUFFER, rendererID);

	// Compute vertex and attribute sizes, first attribute location offsets from buffer start.
	for (auto& spec : specs) {
		attributeSpecs.push_back(spec);
	}
	vertexSize = std::accumulate(attributeSpecs.begin(), attributeSpecs.end(), 0, [&](int sum, const OpenGLVertexSpecification& curr) { return sum + curr.size; });
	DeclareAttributes();

	if (data == nullptr) return;
	// Upload data
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertexSize * numVertices, data, GL_STATIC_DRAW);

}

void OpenGLVertexBuffer::PushAttribute(VertexSpecification& spec) {
	auto glSpec = OpenGLVertexSpecification(spec);
	attributeSpecs.push_back(glSpec);
	vertexSize += glSpec.size;

	DeclareAttributes();
}

void OpenGLVertexBuffer::PopAttribute() {
	assert(attributeSpecs.size() > 0); // should at least have one attribute before popping
	auto& glSpec = attributeSpecs[attributeSpecs.size() - 1];
	attributeSpecs.pop_back();
	vertexSize -= glSpec.size;
	glDisableVertexAttribArray(glSpec.index);

	DeclareAttributes();
}

void OpenGLVertexBuffer::DeclareAttributes() {
	assert(vertexSize > 0); // needs vertexSize computed
	for (unsigned int ix = 0; ix < attributeSpecs.size(); ix++) {
		const auto& spec = attributeSpecs[ix];
		int offset = ix == 0 ? 0 : attributeSpecs[ix - 1].size;
		glVertexAttribPointer(spec.index, spec.numComponents, ALTypeToGLType(spec.type), spec.normalized, vertexSize, (void*)offset);
		glEnableVertexAttribArray(spec.index);
	}
}
