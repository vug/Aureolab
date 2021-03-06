#pragma once

#include "VertexArray.h"

#include <glm/glm.hpp>

#include <unordered_set>

enum class GraphicsAbility {
	Blend, // Blend fragment color with buffer color
	DepthTest, // Update buffer after depth comparison
	StencilTest,
	PointSize, // Use gl_PointSize from shaders
	FaceCulling, // Rendering front/back/both faces
	PolygonOffsetFill,
	PolygonOffsetLine,
	PolygonOffsetPoint,
};

enum class BlendingFactor {
	Zero,
	One,
	SourceAlpha,
	DestinationAlpha,
	OneMinusSourceAlpha,
};

enum class CullFace {
	Back, Front, FrontAndBack,
};
static inline const char* CullFaceNames[] = { "Back", "Front", "FrontAndBack", }; // for GUI

enum class PolygonMode {
	Point, Line, Fill,
};

enum class BufferTestFunction {
	Never, Always, 
	Less, LessThanEqual, GreaterThanEqual, Greater,
	Equal, NotEqual,
};

enum class StencilAction {
	Keep, Zero, Replace, IncrementClamp, InrecementWrap, DecrementClamp, DecrementWrap, BitwiseInvert,
};

enum class ClearableBuffer {
	Color, Depth, Stencil,
};


class GraphicsAPI {
public:
	static GraphicsAPI* Get();

	virtual void SetClearColor(const glm::vec4& color) = 0;
	virtual void Clear(std::unordered_set<ClearableBuffer> buffers = {}) = 0;
	
	virtual void Enable(GraphicsAbility ability) = 0;
	virtual void Disable(GraphicsAbility ability) = 0;
	virtual bool IsEnabled(GraphicsAbility ability) = 0;

	virtual glm::ivec4 GetViewportPositionAndSize() const = 0;

	// Default rasterized point size when PointSize GraphicsAbility is disabled.
	virtual void SetDefaultPointSize(float diameter) = 0;
	virtual void SetBlendingFunction(BlendingFactor src, BlendingFactor dst) = 0;
	virtual void SetCullFace(CullFace cullFace) = 0;
	// Whether to render points, lines or filled faces
	virtual void SetPolygonMode(PolygonMode polygonMode) = 0;
	virtual void SetDepthFunction(BufferTestFunction depthTestFunction) = 0;
	virtual void SetStencilOperation(StencilAction stencilTestFail, StencilAction depthTestFail, StencilAction depthTestPass) = 0;
	virtual void SetStencilFunction(BufferTestFunction stencilTestFunction, int reference, unsigned int mask) = 0;
	virtual void SetStencilMask(unsigned int mask) = 0;
	virtual void SetPolygonOffset(float factor, float units) = 0;

	virtual void DrawIndexedTriangles(const VertexArray& vertexArray, unsigned int indexCount = 0) = 0;
	virtual void DrawIndexedPoints(const VertexArray& vertexArray, unsigned int indexCount = 0) = 0;
	virtual void DrawArrayTriangles(const VertexArray& vertexArray, unsigned int start = 0, unsigned int count = 0) = 0;
	virtual void DrawArrayPoints(const VertexArray& vertexArray, unsigned int start = 0, unsigned int count = 0) = 0;
protected:
	static GraphicsAPI* instance;
};