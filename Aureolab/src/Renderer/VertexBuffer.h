#pragma once

#include "VertexSpecification.h"

#include <any>
#include <cassert>
#include <vector>


class VertexBuffer {
public:
	static VertexBuffer* Create(std::vector<VertexAttributeSpecification> specs);

	template <typename TVertex>
	void SetVertices(const std::vector<TVertex>& newVertices);
	template <typename TVertex>
	void AppendVertices(const std::vector<TVertex>& newVertices);
	template <typename TVertex>
	void AppendVertex(const TVertex& vertex);
	template <typename TVertex>
	void UpdateVertex(unsigned int index, const TVertex& vertex);
	template <typename TVertex>
	void DeleteVertex(unsigned int index);

	virtual const std::vector<VertexAttributeSpecification>& GetAttributeSpecs() const = 0;
	virtual const std::vector<unsigned int> GetAttributeSizes() const = 0;
	virtual const unsigned int GetVertexSize() const = 0;

	virtual void Bind() = 0;
	virtual void Unbind() = 0;

protected:
	// Concrete implementations should provide functionality to upload a void* into buffer.
	virtual void UploadBuffer(size_t size, void* data) = 0;
	template <typename TVertex>
	// Cast the vector blob stored in (void*)vertices member into std::vector<TVertex>
	std::vector<TVertex>* GetVertices();

private:
	template <typename TVertex>
	void UploadVertices();
	// This void* is not the exact content of the buffer, but a pointer to an std::vector<TVertex>.
	// Templated data members require the class itself to be templated. The template, then, disperses to any other classes that interacts with this one.
	// To prevent the whole codebase to be templated with TVertex, instead, have this type unsafe pointer to store the vector as a blob.
	void* vertices = nullptr;
};

// Templated methods are written in this header file to allow typenames to be anything without explicitly state them before usage.
template<typename TVertex>
std::vector<TVertex>* VertexBuffer::GetVertices() {
	if (vertices == nullptr) {
		vertices = (void*)new std::vector<TVertex>();
	}
	return (std::vector<TVertex>*)vertices;
}

template <typename TVertex>
void VertexBuffer::UploadVertices() {
	UploadBuffer(GetVertexSize() * GetVertices<TVertex>()->size(), GetVertices<TVertex>()->data());
}

template<typename TVertex>
void VertexBuffer::SetVertices(const std::vector<TVertex>& newVertices) {
	auto verts = GetVertices<TVertex>();
	*verts = newVertices; // copy operation
	UploadVertices<TVertex>();
}

template<typename TVertex>
void VertexBuffer::AppendVertex(const TVertex& vertex) {
	GetVertices<TVertex>()->push_back(vertex);
	UploadVertices<TVertex>();
}

template<typename TVertex>
void VertexBuffer::AppendVertices(const std::vector<TVertex>& newVertices) {
	auto verts = GetVertices<TVertex>();
	verts->insert(verts->end(), newVertices.begin(), newVertices.end());
	UploadVertices<TVertex>();
}

template<typename TVertex>
void VertexBuffer::UpdateVertex(unsigned int index, const TVertex& vertex) {
	assert(index < GetVertices<TVertex>()->size());
	(*GetVertices<TVertex>())[index] = vertex;
	UploadVertices<TVertex>();
}

template <typename TVertex>
void VertexBuffer::DeleteVertex(unsigned int index) {
	assert(index < GetVertices<TVertex>()->size());
	GetVertices<TVertex>()->erase(GetVertices<TVertex>()->begin() + index);
	UploadVertices();
}