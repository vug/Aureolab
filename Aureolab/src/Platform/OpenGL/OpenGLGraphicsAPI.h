#pragma once

#include "Renderer/GraphicsAPI.h"

class OpenGLGraphicsAPI : public GraphicsAPI {
public:
	OpenGLGraphicsAPI() = default;

	virtual void SetClearColor(const glm::vec4& color) override;
	virtual void Clear(bool colorBuffer = true, bool depthBuffer = true) override;

	virtual void Enable(GraphicsAbility ability) override;
	virtual void Disable(GraphicsAbility ability) override;
	virtual bool IsEnabled(GraphicsAbility ability) override;

	virtual glm::ivec4 GetViewportPositionAndSize() const override;

	virtual void SetDefaultPointSize(float diameter) override;
	virtual void SetBlendingFunction(BlendingFactor src, BlendingFactor dst) override;
	virtual void SetCullFace(CullFace cullFace) override;
	virtual void SetPolygonMode(PolygonMode polygonMode) override;

	virtual void DrawIndexedTriangles(const VertexArray& vertexArray, unsigned int indexCount = 0) override;
	virtual void DrawIndexedPoints(const VertexArray& vertexArray, unsigned int indexCount = 0) override;
	virtual void DrawArrayTriangles(const VertexArray& vertexArray, unsigned int start = 0, unsigned int count = 0) override;
	virtual void DrawArrayPoints(const VertexArray& vertexArray, unsigned int start = 0, unsigned int count = 0) override;
};