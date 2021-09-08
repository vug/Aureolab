#pragma once

#include "Renderer/VertexArray.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"

class OpenGLVertexArray : public VertexArray {
public:
	OpenGLVertexArray();
	virtual ~OpenGLVertexArray();

	virtual void Bind() override;
	virtual void Unbind() override;

	virtual void AddVertexBuffer(const VertexBuffer& vertexBuffer) override;
	virtual void SetIndexBuffer(const IndexBuffer& indexBuffer) override;
private:
	unsigned int rendererID = -1;
};