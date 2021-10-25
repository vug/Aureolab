#include "OpenGLGraphicsAPI.h"

#include <glad/glad.h>

GLenum AbilityAL2GL(GraphicsAbility ability) {
	switch (ability) {
	case GraphicsAbility::Blend:
		return GL_BLEND;
	case GraphicsAbility::PointSize:
		return GL_PROGRAM_POINT_SIZE;
	case GraphicsAbility::DepthTest:
		return GL_DEPTH_TEST;
	case GraphicsAbility::StencilTest:
		return GL_STENCIL_TEST;
	case GraphicsAbility::FaceCulling:
		return GL_CULL_FACE;
	case GraphicsAbility::PolygonOffsetFill:
		return GL_POLYGON_OFFSET_FILL;
	case GraphicsAbility::PolygonOffsetLine:
		return GL_POLYGON_OFFSET_LINE;
	case GraphicsAbility::PolygonOffsetPoint:
		return GL_POLYGON_OFFSET_POINT;
	default:
		assert(false); // unknown GraphicsAbility
		return -1;
	}
}

GLenum BlendingFactorAL2GL(BlendingFactor bf) {
	switch (bf) {
	case BlendingFactor::Zero:
		return GL_ZERO;
	case BlendingFactor::One:
		return GL_ONE;
	case BlendingFactor::SourceAlpha:
		return GL_SRC_ALPHA;
	case BlendingFactor::OneMinusSourceAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	case BlendingFactor::DestinationAlpha:
		return GL_DST_ALPHA;
	default:
		assert(false); // unknown BlendingFactor
		return -1;
	}
}

GLenum cullFaceAL2GL(CullFace cf) {
	switch (cf) {
	case CullFace::Back:
		return GL_BACK;
	case CullFace::Front:
		return GL_FRONT;
	case CullFace::FrontAndBack:
		return GL_FRONT_AND_BACK;
	default:
		assert(false); // unknown BlendingFactor
		return -1;
	}
}

inline GLenum PolygonModeAL2GL(PolygonMode pm) {
	switch (pm) {
	case PolygonMode::Fill:
		return GL_FILL;
	case PolygonMode::Line:
		return GL_LINE;
	case PolygonMode::Point:
		return GL_POINT;
	default:
		assert(false); // unknown PolygonMode
		return -1;
	}
}

inline GLenum BufferTestFunctionAL2GL(BufferTestFunction df) {
	switch (df) {
	case BufferTestFunction::Never:
		return GL_NEVER;
	case BufferTestFunction::Always:
		return GL_ALWAYS;
	case BufferTestFunction::Less:
		return GL_LESS;
	case BufferTestFunction::LessThanEqual:
		return GL_LEQUAL;
	case BufferTestFunction::GreaterThanEqual:
		return GL_GEQUAL;
	case BufferTestFunction::Greater:
		return GL_GREATER;
	case BufferTestFunction::Equal:
		return GL_EQUAL;
	case BufferTestFunction::NotEqual:
		return GL_NOTEQUAL;
	default:
		assert(false); // unknown DepthTestFunction
		return -1;
	}
}

inline GLenum StencilActionAL2GL(StencilAction sa) {
	switch (sa) {
	default:
		case StencilAction::Keep:
			return GL_KEEP;
		case StencilAction::Zero:
			return GL_ZERO;
		case StencilAction::Replace:
			return GL_REPLACE;
		case StencilAction::IncrementClamp:
			return GL_INCR;
		case StencilAction::InrecementWrap:
			return GL_INCR_WRAP;
		case StencilAction::DecrementClamp:
			return GL_DECR;
		case StencilAction::DecrementWrap:
			return GL_DECR_WRAP;
		case StencilAction::BitwiseInvert:
			return GL_INVERT;
		assert(false); // unknown StencilAction
		return -1;
	}
}


void OpenGLGraphicsAPI::SetClearColor(const glm::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLGraphicsAPI::Clear(std::unordered_set<ClearableBuffer> buffers) {
	if (buffers.empty()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	else {
		GLbitfield mask = 0x00000000;
		if (buffers.contains(ClearableBuffer::Color)) { mask |= GL_COLOR_BUFFER_BIT; }
		if (buffers.contains(ClearableBuffer::Depth)) { mask |= GL_DEPTH_BUFFER_BIT; }
		if (buffers.contains(ClearableBuffer::Stencil)) { mask |= GL_STENCIL_BUFFER_BIT; }
		glClear(mask);
	}
}

void OpenGLGraphicsAPI::Enable(GraphicsAbility ability) {
	GLenum glEnum = AbilityAL2GL(ability);
	glEnable(glEnum);
}

void OpenGLGraphicsAPI::Disable(GraphicsAbility ability) {
	GLenum glEnum = AbilityAL2GL(ability);
	glDisable(glEnum);
}

bool OpenGLGraphicsAPI::IsEnabled(GraphicsAbility ability) {
	GLenum glEnum = AbilityAL2GL(ability);
	return glIsEnabled(glEnum);
}

glm::ivec4 OpenGLGraphicsAPI::GetViewportPositionAndSize() const {
	int data[4];
	glGetIntegerv(GL_VIEWPORT, data);
	return { data[0], data[1], data[2], data[3] };
}

void OpenGLGraphicsAPI::SetDefaultPointSize(float diameter) {
	glPointSize(diameter);
}

void OpenGLGraphicsAPI::SetBlendingFunction(BlendingFactor src, BlendingFactor dst) {
	GLenum srcEnum = BlendingFactorAL2GL(src);
	GLenum dstEnum = BlendingFactorAL2GL(dst);
	glBlendFunc(srcEnum, dstEnum);
}

void OpenGLGraphicsAPI::SetCullFace(CullFace cullFace) {
	GLenum glEnum = cullFaceAL2GL(cullFace);
	glCullFace(glEnum);
}

void OpenGLGraphicsAPI::SetPolygonMode(PolygonMode polygonMode) {
	GLenum glEnum = PolygonModeAL2GL(polygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, glEnum);
}

void OpenGLGraphicsAPI::SetDepthFunction(BufferTestFunction depthTestFunction) {
	GLenum glEnum = BufferTestFunctionAL2GL(depthTestFunction);
	glDepthFunc(glEnum);
}

void OpenGLGraphicsAPI::SetStencilOperation(StencilAction stencilTestFail, StencilAction depthTestFail, StencilAction depthTestPass) {
	GLenum fail = StencilActionAL2GL(stencilTestFail);
	GLenum zfail = StencilActionAL2GL(depthTestFail);
	GLenum zpass = StencilActionAL2GL(depthTestPass);
	glStencilOp(fail, zfail, zpass);
}

void OpenGLGraphicsAPI::SetStencilFunction(BufferTestFunction stencilTestFunction, int reference, unsigned int mask) {
	GLenum func = BufferTestFunctionAL2GL(stencilTestFunction);
	glStencilFunc(func, reference, mask);
}

void OpenGLGraphicsAPI::SetStencilMask(unsigned int mask) {
	glStencilMask(mask);
}

void OpenGLGraphicsAPI::SetPolygonOffset(float factor, float units) {
	glPolygonOffset(factor, units);
}

void OpenGLGraphicsAPI::DrawIndexedTriangles(const VertexArray& vertexArray, unsigned int indexCount) {
	vertexArray.Bind();
	indexCount = indexCount == 0 ? (unsigned int)vertexArray.GetIndexBuffer()->GetNumIndices() : indexCount;
	glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, 0);
}

void OpenGLGraphicsAPI::DrawIndexedPoints(const VertexArray& vertexArray, unsigned int indexCount) {
	vertexArray.Bind();
	indexCount = indexCount == 0 ? (unsigned int)vertexArray.GetIndexBuffer()->GetNumIndices() : indexCount;
	glDrawElements(GL_POINTS, (GLsizei)indexCount, GL_UNSIGNED_INT, 0);
}

void OpenGLGraphicsAPI::DrawArrayTriangles(const VertexArray& vertexArray, unsigned int start, unsigned int count) {
	vertexArray.Bind();
	count = count == 0 ? vertexArray.GetVertexBuffers()[0]->GetNumVertices() : count;
	glDrawArrays(GL_TRIANGLES, start, count);
}

void OpenGLGraphicsAPI::DrawArrayPoints(const VertexArray& vertexArray, unsigned int start, unsigned int count) {
	vertexArray.Bind();
	count = count == 0 ? vertexArray.GetVertexBuffers()[0]->GetNumVertices() : count;
	glDrawArrays(GL_POINTS, start, count);
}

