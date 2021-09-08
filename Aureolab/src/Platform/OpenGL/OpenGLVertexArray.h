#pragma once

#include "Renderer/VertexArray.h"

#include "Renderer/VertexBuffer.h"

class OpenGLVertexArray : public VertexArray {

	virtual void AddVertexBuffer(const VertexBuffer& vertexBuffer) override;
};