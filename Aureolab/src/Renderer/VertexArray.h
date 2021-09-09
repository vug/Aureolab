#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <vector>

class VertexArray {
public:
	static VertexArray* Create();
	virtual ~VertexArray() = default;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	virtual void AddVertexBuffer(const VertexBuffer& vertexBuffer) = 0;
	virtual void SetIndexBuffer(const IndexBuffer& indexBuffer) = 0;
	virtual const std::vector<const VertexBuffer*>& GetVertexBuffers() const = 0;
	virtual const IndexBuffer* GetIndexBuffer() const = 0;
};