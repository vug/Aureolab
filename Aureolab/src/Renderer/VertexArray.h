#pragma once

#include "VertexBuffer.h"

class VertexArray {
public:
	static VertexArray* Create();
	virtual ~VertexArray() = default;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	virtual void AddVertexBuffer(const VertexBuffer& vertexBuffer) = 0;
	//virtual void SetIndexBuffer(const IndexBuffer& indexBuffer) = 0;
};