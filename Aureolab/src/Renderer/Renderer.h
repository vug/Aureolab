#pragma once

#include "Scene/Components.h"
#include "Renderer/Shader.h"

#include <glm/glm.hpp>
#include <entt/entt.hpp>

struct ViewData {
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::vec4 viewPosition;
};

#define MAX_LIGHTS 10

struct PointLight {
	glm::vec4 attenuation; // vec3
	glm::vec4 position; // vec3
};
struct DirectionalLight {
	glm::vec4 direction; // vec3
};
struct Light {
	glm::vec4 color;
	PointLight pointParams;
	DirectionalLight directionalParams;
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
	static void RenderMesh(Shader* shader, const ViewData& viewData, const TransformComponent& transform, const MeshComponent& mesh, const MeshRendererComponent& meshRenderer);
	static void RenderProceduralMesh(Shader* shader, const ViewData& viewData, const TransformComponent& transform, const ProceduralMeshComponent& mesh, const MeshRendererComponent& meshRenderer);
	static void RenderVertexArray(Shader* shader, const ViewData& viewData, const TransformComponent& transform, VertexArray* vao, const MeshRendererComponent& meshRenderer);

	static void RenderVertexArrayEntityID(entt::entity ent, Shader* shader, const ViewData& viewData, const TransformComponent& transform, VertexArray* vao, const MeshRendererComponent& meshRenderer);
};