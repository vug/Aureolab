#pragma once

#include "Renderer/UniformBuffer.h"

class OpenGLUniformBuffer : public UniformBuffer {
public:
	OpenGLUniformBuffer(const std::string& name, unsigned int size);

	virtual void BlockBind(Shader* shader) override;
	virtual void UploadData(const void* data) override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

private:
	unsigned int rendererID = -1;
	std::string name;
	unsigned int size = -1;
	int bindingPoint = -1;
};