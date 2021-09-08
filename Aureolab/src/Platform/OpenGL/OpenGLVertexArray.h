#pragma once

#include "Renderer/VertexArray.h"
#include "Renderer/VertexBuffer.h"

class OpenGLVertexArray : public VertexArray {
public:
	OpenGLVertexArray();

	virtual void Bind() override;
	virtual void Unbind() override;

	virtual void AddVertexBuffer(const VertexBuffer& vertexBuffer) override;
private:
	unsigned int rendererID = -1;
};