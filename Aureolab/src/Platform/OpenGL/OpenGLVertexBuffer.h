#pragma once

#include "Renderer/VertexBuffer.h"

#include <vector>

class OpenGLVertexSpecification : public VertexSpecification {
public:
	OpenGLVertexSpecification(const VertexSpecification& spec);
	unsigned int size; // Size of 
};

class OpenGLVertexBuffer : public VertexBuffer {
public:
	OpenGLVertexBuffer();
	OpenGLVertexBuffer(std::vector<VertexSpecification> specs, unsigned int numVertices = 0, void* data = nullptr);

	void PushAttribute(VertexSpecification& spec);
	void PopAttribute();
private:
	unsigned int rendererID = -1;
	unsigned int vertexSize = 0; // aka stride. total size of all attributes in bytes.
	std::vector<OpenGLVertexSpecification> attributeSpecs = {};
	void DeclareAttributes();
};