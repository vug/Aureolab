#pragma once

#include "Core/Log.h"
#include "Renderer/VertexBuffer.h"

#include <glad/glad.h>

#include <vector>

class OpenGLVertexBuffer : public VertexBuffer {
public:
	OpenGLVertexBuffer(const std::vector<VertexSpecification>& specs);

	virtual unsigned int GetVertexSize() override;
	virtual void Bind() override;
	virtual void Unbind() override;
private:
	unsigned int rendererID = -1;
	unsigned int vertexSize = 0; // aka stride. total size of all attributes in bytes.
	std::vector<VertexSpecification> attributeSpecs;

	virtual void UploadBuffer(size_t size, void* data) override;
};

