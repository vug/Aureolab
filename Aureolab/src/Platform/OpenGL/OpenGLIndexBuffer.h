#pragma once

#include "Renderer/IndexBuffer.h"

#include <vector>

class OpenGLIndexBuffer : public IndexBuffer {
public:
	OpenGLIndexBuffer();
	virtual ~OpenGLIndexBuffer();

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void UploadIndices(std::vector<unsigned int> indices) override;
private:
	unsigned int rendererID = -1;
};