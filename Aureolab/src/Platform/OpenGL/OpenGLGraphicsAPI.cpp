#include "OpenGLGraphicsAPI.h"

#include <glad/glad.h>

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

