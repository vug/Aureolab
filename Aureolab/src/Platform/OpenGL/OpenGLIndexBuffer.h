#pragma once

#include "Renderer/IndexBuffer.h"

#include <vector>

class OpenGLIndexBuffer : public IndexBuffer {
public:
	OpenGLIndexBuffer();
	virtual ~OpenGLIndexBuffer();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void UploadIndices(const std::vector<unsigned int>& indices) override;
	virtual const size_t GetNumIndices() const override { return numIndices; };
private:
	unsigned int rendererID = -1;
	size_t numIndices = 0;
};