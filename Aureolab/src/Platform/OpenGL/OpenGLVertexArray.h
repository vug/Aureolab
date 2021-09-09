#pragma once

#include "Renderer/VertexArray.h"
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"

#include <vector>

class OpenGLVertexArray : public VertexArray {
public:
	OpenGLVertexArray();
	virtual ~OpenGLVertexArray();

	virtual void Bind() override;
	virtual void Unbind() override;

	virtual void AddVertexBuffer(const VertexBuffer& vertexBuffer) override;
	virtual void SetIndexBuffer(const IndexBuffer& indexBuffer) override;
	virtual const std::vector<const VertexBuffer*>& GetVertexBuffers() const override { return vertexBuffers; };
	virtual const IndexBuffer* GetIndexBuffer() const override { return indexBuffer; }
private:
	unsigned int rendererID = -1;
	std::vector<const VertexBuffer*> vertexBuffers = {};
	const IndexBuffer* indexBuffer = nullptr;
};