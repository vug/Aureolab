#pragma once

#include "Renderer/VertexArray.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"

#include <vector>

class OpenGLVertexArray : public VertexArray {
public:
	OpenGLVertexArray();
	virtual ~OpenGLVertexArray();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual unsigned int GetRendererID() const { return rendererID; }

	virtual void AddVertexBuffer(VertexBuffer& vertexBuffer) override;
	virtual void SetIndexBuffer(const IndexBuffer& indexBuffer) override;
	virtual const std::vector<VertexBuffer*>& GetVertexBuffers() const override { return vertexBuffers; };
	virtual const IndexBuffer* GetIndexBuffer() const override { return indexBuffer; }
private:
	unsigned int rendererID = -1;
	std::vector<VertexBuffer*> vertexBuffers = {};
	const IndexBuffer* indexBuffer = nullptr;
};