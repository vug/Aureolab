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

void OpenGLGraphicsAPI::Initialize() {
}

void OpenGLGraphicsAPI::SetClearColor(const glm::vec4& color) {
	glClearColor(color.r, color.b, color.g, color.a);
}

void OpenGLGraphicsAPI::Clear(bool colorBuffer, bool depthBuffer) {
	GLbitfield mask = 0x00000000;
	if (colorBuffer) mask |= GL_COLOR_BUFFER_BIT;
	if (depthBuffer) mask |= GL_DEPTH_BUFFER_BIT;
	glClear(mask);
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

void OpenGLGraphicsAPI::SetDefaultPointSize(float diameter) {
	glPointSize(diameter);
}

void OpenGLGraphicsAPI::SetBlendingFunction(BlendingFactor src, BlendingFactor dst) {
	GLenum srcEnum = BlendingFactorAL2GL(src);
	GLenum dstEnum = BlendingFactorAL2GL(dst);
	glBlendFunc(srcEnum, dstEnum);
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

void OpenGLGraphicsAPI::DrawArrayPoints(const VertexArray& vertexArray, unsigned int start, unsigned int count) {
	vertexArray.Bind();
	count = count == 0 ? vertexArray.GetVertexBuffers()[0]->GetNumVertices() : count;
	glDrawArrays(GL_POINTS, start, count);
}

