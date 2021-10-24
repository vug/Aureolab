#pragma once

#include "Scene/Components.h"
#include "Renderer/Shader.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>

struct ViewMatrices {
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::vec4 viewPosition;
};

#define MAX_LIGHTS 10

struct PointLight {
	glm::vec4 attenuation;
};

struct Light {
	glm::vec4 position;
	glm::vec4 color;
	PointLight pointParams;
	float intensity;
	int type;
	float _pad3;
	float _pad4;
};

struct Lights {
	Light lights[MAX_LIGHTS];
	glm::vec4 ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	int numLights;
};


class Renderer {
public:
	static void RenderMesh(Shader* shader, const ViewMatrices& viewMatrices, const TransformComponent& transform, const MeshComponent& mesh, const MeshRendererComponent& meshRenderer);
	static void RenderProceduralMesh(Shader* shader, const ViewMatrices& viewMatrices, const TransformComponent& transform, const ProceduralMeshComponent& mesh, const MeshRendererComponent& meshRenderer);
	static void RenderVertexArray(Shader* shader, const ViewMatrices& viewMatrices, const TransformComponent& transform, VertexArray* vao, const MeshRendererComponent& meshRenderer);

	static void RenderVertexArrayEntityID(entt::entity ent, Shader* shader, const ViewMatrices& viewMatrices, const TransformComponent& transform, VertexArray* vao, const MeshRendererComponent& meshRenderer);
};