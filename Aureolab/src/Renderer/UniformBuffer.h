#pragma once

#include "Renderer/Shader.h"

#include <string>

class UniformBuffer {
public:
	static UniformBuffer* Create(const std::string& name, unsigned int size);

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void BlockBind(Shader* shader) = 0;
	virtual void UploadData(const void* data) = 0;

protected:
	static int bindingPointCounter;
};