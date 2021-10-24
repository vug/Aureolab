#include "Renderer.h"

#include "Core/Math.h"
#include "Renderer/GraphicsAPI.h"
#include "Renderer/Shader.h"

void Renderer::RenderMesh(Shader* shader, const ViewData& viewData, const TransformComponent& transform, const MeshComponent& mesh, const MeshRendererComponent& meshRenderer) {
	Renderer::RenderVertexArray(shader, viewData, transform, mesh.vao, meshRenderer);
}

void Renderer::RenderProceduralMesh(Shader* shader, const ViewData& viewData, const TransformComponent& transform, const ProceduralMeshComponent& pMesh, const MeshRendererComponent& meshRenderer) {
	Renderer::RenderVertexArray(shader, viewData, transform, pMesh.vao, meshRenderer);
}

void Renderer::RenderVertexArray(Shader* shader, const ViewData& viewData, const TransformComponent& transform, VertexArray* vao, const MeshRendererComponent& meshRenderer) {
	const glm::vec3& translation = transform.translation;
	const glm::mat4 model = Math::ComposeTransform(transform.translation, transform.rotation, transform.scale);
	const glm::mat4 modelView = viewData.view * model;
	const glm::mat4 modelViewProjection = viewData.projection * modelView;
	const glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));
	shader->UploadUniformMat4("u_Model", model);
	shader->UploadUniformMat4("u_ModelView", modelView);
	shader->UploadUniformMat4("u_ModelViewPerspective", modelViewProjection);
	shader->UploadUniformMat4("u_NormalMatrix", normalMatrix);

	shader->UploadUniformInt("u_RenderType", (int)meshRenderer.visualization);
	shader->UploadUniformFloat4("u_SolidColor", meshRenderer.solidColor);
	shader->UploadUniformFloat4("u_ObjectColor", meshRenderer.objectColor);
	shader->UploadUniformFloat("u_DepthMax", meshRenderer.depthParams.max);
	shader->UploadUniformFloat("u_DepthPow", meshRenderer.depthParams.pow);

	if (vao == nullptr) { return; }
	GraphicsAPI::Get()->DrawArrayTriangles(*vao);
}

void Renderer::RenderVertexArrayEntityID(entt::entity ent, Shader* shader, const ViewData& viewData, const TransformComponent& transform, VertexArray* vao, const MeshRendererComponent& meshRenderer) {
	const glm::vec3& translation = transform.translation;
	const glm::mat4 model = Math::ComposeTransform(transform.translation, transform.rotation, transform.scale);
	const glm::mat4 modelView = viewData.view * model;
	const glm::mat4 modelViewProjection = viewData.projection * modelView;
	const glm::mat4 normalMatrix = glm::inverse(modelView);
	shader->UploadUniformMat4("u_ModelViewPerspective", modelViewProjection);
	shader->UploadUniformInt("u_EntityID", (int)ent);
	if (vao == nullptr) { return; }
	GraphicsAPI::Get()->DrawArrayTriangles(*vao);
}
