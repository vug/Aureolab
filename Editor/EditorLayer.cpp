#include "EditorLayer.h"

#include "ExampleScene.h"

#include "Core/GraphicsContext.h"
#include "Renderer/GraphicsAPI.h"
#include "Core/Math.h"
#include "Scene/Components.h"
#include "Renderer/Renderer.h"

#include <imgui.h>
#include <glm/glm.hpp>

#include <string>

void EditorLayer::OnAttach() {
	// in case we'll see an area not behind any ImWindow
	GraphicsAPI::Get()->Enable(GraphicsAbility::DepthTest);
	auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
	GraphicsAPI::Get()->SetClearColor(turquoise);

	selectionShader = Shader::Create("assets/shaders/SelectionShader.glsl");
	shader = Shader::Create("assets/shaders/BasicShader.glsl");
	viewportFbo = FrameBuffer::Create(100, 100, FrameBuffer::TextureFormat::RGBA8); // arguments does not matter since FBO's going to be resized
	selectionFbo = FrameBuffer::Create(100, 100, FrameBuffer::TextureFormat::RED_INTEGER);
	camera = new EditorCamera(45, 1.0f, 0.01f, 100); // aspect = 1.0f will be recomputed

	ExampleScene::PopulateScene(scene);
}


void EditorLayer::OnUpdate(float ts) {
	std::shift_left(frameRates.begin(), frameRates.end(), 1);
	frameRates[frameRates.size() - 1] = 1.0f / ts;

	camera->OnUpdate(ts);
	const glm::mat4& projection = camera->GetProjection();
	const glm::mat4& view = camera->GetViewMatrix();

	GraphicsAPI::Get()->Clear();

	// Render pass 1: render scene into viewportFBO
	viewportFbo->Bind();
	GraphicsAPI::Get()->SetClearColor({ 0, 0, 0, 1 });
	GraphicsAPI::Get()->Clear();
	shader->Bind();
	auto query = scene.View<TransformComponent, MeshComponent, MeshRendererComponent>();
	for (const auto& [ent, transform, mesh, meshRenderer] : query.each()) {
		Renderer::RenderMesh(shader, view, projection, transform, mesh, meshRenderer);
	}
	auto query2 = scene.View<TransformComponent, ProceduralMeshComponent, MeshRendererComponent>();
	for (const auto& [ent, transform, pMesh, meshRenderer] : query2.each()) {
		Renderer::RenderProceduralMesh(shader, view, projection, transform, pMesh, meshRenderer);
	}
	shader->Unbind();
	viewportFbo->Unbind();

	// Render pass 2: render entityID of each object into an integer buffer
	selectionFbo->Bind();
	selectionFbo->Clear(-1); // value when not hovering on any object
	selectionShader->Bind();
	for (const auto& [ent, transform, mesh, meshRenderer] : query.each()) {
		Renderer::RenderVertexArrayEntityID(ent, selectionShader, view, projection, transform, mesh.vao, meshRenderer);
	}
	for (const auto& [ent, transform, pMesh, meshRenderer] : query2.each()) {
		Renderer::RenderVertexArrayEntityID(ent, selectionShader, view, projection, transform, pMesh.vao, meshRenderer);
	}
	hoveredEntityId = -3; // value when queried coordinates are not inside the selectionFbo
	selectionFbo->ReadPixel(hoveredEntityId, mouseX, mouseY);
	hoveredObject = scene.GetHandle((entt::entity)hoveredEntityId);
	selectionShader->Unbind();
	selectionFbo->Unbind();
}

void EditorLayer::OnDetach() {
	delete viewportFbo;
	delete selectionFbo;
}

void EditorLayer::OnEvent(Event& ev) {
	viewportPanel.OnEvent(ev);
}

void EditorLayer::OnImGuiRender() {
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_None);

	mainMenuBar.OnImGuiRender();
	hierarchyPanel.OnImGuiRender(); // Left column
	viewportPanel.OnImGuiRender(); // Middle column
	inspectorPanel.OnImGuiRender(); // Right column

	ImGui::Begin("Stats");
	ImGui::Text("Stats:\n"
		"mainViewportSize: (%.1f, %.1f)\n"
		"",
		viewport->Size.x, viewport->Size.y
	);

	if (ImGui::CollapsingHeader("FPS", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PlotLines("", frameRates.data(), (int)frameRates.size());
		const auto [minIt, maxIt] = std::minmax_element(frameRates.begin(), frameRates.end());
		ImGui::Text("[%.1f %.1f]", *minIt, *maxIt);
		static bool isVSync = false;
		if (ImGui::Checkbox("VSync", &isVSync)) { GraphicsContext::Get()->SetVSync(isVSync); }
	}

	if (ImGui::CollapsingHeader("Editor Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Yaw, Pitch, Roll: (%.2f, %.2f, %.2f)", camera->GetYaw(), camera->GetPitch(), camera->GetRoll());
		ImGui::Text("Pos: (%.1f, %.1f, %.1f)", camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
		ImGui::Text("Target: (%.1f, %.1f, %.1f)", camera->GetFocalPoint().x, camera->GetFocalPoint().y, camera->GetFocalPoint().z);
		ImGui::Text("Distance: %.1f", camera->GetDistance());
		float fov = camera->GetFOV();
		if (ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f)) { camera->SetFOV(fov); }
		ImGui::SliderFloat("Roll", camera->GetRefRoll(), 0.0f, 3.141593f);
	}

	ImGui::Separator();
	static bool shouldShowDemo = false;
	ImGui::Checkbox("Show Demo Window", &shouldShowDemo);
	ImGui::End();

	if (shouldShowDemo) ImGui::ShowDemoWindow();
}

