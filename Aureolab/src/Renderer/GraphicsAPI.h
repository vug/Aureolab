#pragma once

#include "VertexArray.h"

#include <glm/glm.hpp>

class GraphicsAPI {
public:
	static GraphicsAPI* Create();

	virtual void Initialize() = 0;

	virtual void SetClearColor(const glm::vec4& color) = 0;
	virtual void Clear(bool colorBuffer = true, bool depthBuffer = true) = 0;

	virtual void DrawIndexedTriangles(const VertexArray& vertexArray, unsigned int indexCount = 0) = 0;
};