#pragma once

#include "Scene/Components.h"
#include "Renderer/Shader.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>

class Renderer {
public:
	static void RenderMesh(Shader* shader, const glm::mat4& view, const glm::mat4& projection, const TransformComponent& transform, const MeshComponent& mesh, const MeshRendererComponent& meshRenderer);
	static void RenderProceduralMesh(Shader* shader, const glm::mat4& view, const glm::mat4& projection, const TransformComponent& transform, const ProceduralMeshComponent& mesh, const MeshRendererComponent& meshRenderer);
	static void RenderVertexArray(Shader* shader, const glm::mat4& view, const glm::mat4& projection, const TransformComponent& transform, VertexArray* vao, const MeshRendererComponent& meshRenderer);

	static void RenderVertexArrayEntityID(entt::entity ent, Shader* shader, const glm::mat4& view, const glm::mat4& projection, const TransformComponent& transform, VertexArray* vao, const MeshRendererComponent& meshRenderer);
};