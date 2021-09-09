#pragma once

#include "Renderer/GraphicsAPI.h"

class OpenGLGraphicsAPI : public GraphicsAPI {
public:
	OpenGLGraphicsAPI() = default;

	virtual void Initialize() override;

	virtual void SetClearColor(const glm::vec4& color) override;
	virtual void Clear(bool colorBuffer = true, bool depthBuffer = true) override;

	virtual void DrawIndexedTriangles(const VertexArray& vertexArray, unsigned int indexCount = 0) override;
	virtual void DrawIndexedPoints(const VertexArray& vertexArray, unsigned int indexCount = 0) override;
	virtual void DrawArrayPoints(const VertexArray& vertexArray, unsigned int start = 0, unsigned int count = 0) override;
};