#pragma once

#include <vector>

class IndexBuffer {
public:
	static IndexBuffer* Create();
	virtual ~IndexBuffer() = default;

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void UploadIndices(const std::vector<unsigned int>& indices) = 0;
	virtual const size_t GetNumIndices() const = 0;
};