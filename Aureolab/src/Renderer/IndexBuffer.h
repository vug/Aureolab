#pragma once

#include <vector>

class IndexBuffer {
public:
	static IndexBuffer* Create();
	virtual ~IndexBuffer() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void UploadIndices(std::vector<unsigned int> indices) = 0;
};