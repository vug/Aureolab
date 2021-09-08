#pragma once

#include "Core/Log.h"
#include "Renderer/VertexBuffer.h"

#include <glad/glad.h>

#include <vector>


class OpenGLVertexSpecification : public VertexSpecification {
public:
	OpenGLVertexSpecification(const VertexSpecification& spec);
	unsigned int size; // Size of 
};


class OpenGLVertexBuffer : public VertexBuffer {
public:
	OpenGLVertexBuffer(std::vector<VertexSpecification> specs);

	virtual unsigned int GetVertexSize() override;
	virtual void Bind() override;
	virtual void Unbind() override;
private:
	unsigned int rendererID = -1;
	unsigned int vertexSize = 0; // aka stride. total size of all attributes in bytes.
	std::vector<OpenGLVertexSpecification> attributeSpecs = {};

	virtual void UploadBuffer(size_t size, void* data) override;
};

