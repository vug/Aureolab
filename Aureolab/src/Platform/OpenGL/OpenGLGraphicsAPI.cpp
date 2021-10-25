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
	case GraphicsAbility::FaceCulling:
		return GL_CULL_FACE;
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

inline GLenum DepthTestFunctionAL2GL(DepthTestFunction df) {
	switch (df) {
	case DepthTestFunction::Never:
		return GL_NEVER;
	case DepthTestFunction::Always:
		return GL_ALWAYS;
	case DepthTestFunction::Less:
		return GL_LESS;
	case DepthTestFunction::LessThanEqual:
		return GL_LEQUAL;
	case DepthTestFunction::GreaterThanEqual:
		return GL_GEQUAL;
	case DepthTestFunction::Greater:
		return GL_GREATER;
	case DepthTestFunction::Equal:
		return GL_EQUAL;
	case DepthTestFunction::NotEqual:
		return GL_NOTEQUAL;
	default:
		assert(false); // unknown DepthTestFunction
		return -1;
	}
}


void OpenGLGraphicsAPI::SetClearColor(const glm::vec4& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLGraphicsAPI::Clear(std::unordered_set<ClearableBuffer> buffers) {
	if (buffers.empty()) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

void OpenGLGraphicsAPI::SetDepthFunction(DepthTestFunction depthTestFunction) {
	GLenum glEnum = DepthTestFunctionAL2GL(depthTestFunction);
	glDepthFunc(glEnum);
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

