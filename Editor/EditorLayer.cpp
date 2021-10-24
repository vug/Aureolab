#include "EditorLayer.h"

#include "ExampleScene.h"

#include "Core/GraphicsContext.h"
#include "Renderer/GraphicsAPI.h"
#include "Scene/Components.h"
#include "Renderer/Renderer.h"

#include <imgui.h>
#include <glm/glm.hpp>

#include <string>

void EditorLayer::OnAttach() {
	// in case we'll see an area not behind any ImWindow
	auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
	GraphicsAPI::Get()->SetClearColor(turquoise);
	GraphicsAPI::Get()->Enable(GraphicsAbility::DepthTest);
	GraphicsAPI::Get()->Enable(GraphicsAbility::FaceCulling);

	selectionShader = Shader::Create("assets/shaders/SelectionShader.glsl");
	shader = Shader::Create("assets/shaders/BasicShader.glsl");
	viewUbo = UniformBuffer::Create("ViewMatrices", sizeof(ViewMatrices));
	viewUbo->BlockBind(shader);
	lightsUbo = UniformBuffer::Create("Lights", sizeof(Lights));
	lightsUbo->BlockBind(shader);
	viewportFbo = FrameBuffer::Create(100, 100, FrameBuffer::TextureFormat::RGBA8); // arguments does not matter since FBO's going to be resized
	selectionFbo = FrameBuffer::Create(100, 100, FrameBuffer::TextureFormat::RED_INTEGER);
	camera = new EditorCamera(45, 1.0f, 0.01f, 100); // aspect = 1.0f will be recomputed

	ExampleScene::PopulateScene(scene);
}


void EditorLayer::OnUpdate(float ts) {
	std::shift_left(frameRates.begin(), frameRates.end(), 1);
	frameRates[frameRates.size() - 1] = 1.0f / ts;

	camera->OnUpdate(ts);
	ViewMatrices viewMatrices;
	viewMatrices.projection = camera->GetProjection();
	viewMatrices.view = camera->GetViewMatrix();
	viewUbo->UploadData((const void*)&viewMatrices);

	// Lights System
	auto queryLights = scene.View<TransformComponent, LightComponent>();
	int ix = 0;
	Lights lightsData;
	for (const auto& [ent, transform, lightC] : queryLights.each()) {
		Light light;
		light.type = (int)lightC.type;
		light.position = { transform.translation.x, transform.translation.y, transform.translation.z, 0.0f };
		light.color = { lightC.color.x, lightC.color.y, lightC.color.z, 0.0f };
		light.intensity = lightC.intensity;
		light.pointParams.attenuation = { lightC.pointParams.attenuation.x, lightC.pointParams.attenuation.y, lightC.pointParams.attenuation.z, 0.0f };
		lightsData.lights[ix] = light;
		ix++;
		if (ix == MAX_LIGHTS) break;
	}
	lightsData.numLights = ix;
	lightsUbo->UploadData((const void*)&lightsData);

	GraphicsAPI::Get()->Clear();

	// Render pass 1: render scene into viewportFBO
	viewportFbo->Bind();
	GraphicsAPI::Get()->SetClearColor({ 0, 0, 0, 1 });
	GraphicsAPI::Get()->Clear();
	shader->Bind();
	auto query = scene.View<TransformComponent, MeshComponent, MeshRendererComponent>();
	for (const auto& [ent, transform, mesh, meshRenderer] : query.each()) {
		Renderer::RenderMesh(shader, viewMatrices, transform, mesh, meshRenderer);
	}
	auto query2 = scene.View<TransformComponent, ProceduralMeshComponent, MeshRendererComponent>();
	for (const auto& [ent, transform, pMesh, meshRenderer] : query2.each()) {
		Renderer::RenderProceduralMesh(shader, viewMatrices, transform, pMesh, meshRenderer);
	}
	shader->Unbind();
	viewportFbo->Unbind();

	// Render pass 2: render entityID of each object into an integer buffer
	selectionFbo->Bind();
	selectionFbo->Clear(-1); // value when not hovering on any object
	selectionShader->Bind();
	for (const auto& [ent, transform, mesh, meshRenderer] : query.each()) {
		Renderer::RenderVertexArrayEntityID(ent, selectionShader, viewMatrices, transform, mesh.vao, meshRenderer);
	}
	for (const auto& [ent, transform, pMesh, meshRenderer] : query2.each()) {
		Renderer::RenderVertexArrayEntityID(ent, selectionShader, viewMatrices, transform, pMesh.vao, meshRenderer);
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
	if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		static bool shouldCullFaces = false;
		if (ImGui::Checkbox("Face Culling", &shouldCullFaces)) {
			if (shouldCullFaces) { GraphicsAPI::Get()->Enable(GraphicsAbility::FaceCulling); }
			else { GraphicsAPI::Get()->Disable(GraphicsAbility::FaceCulling); }
		}

		if (shouldCullFaces) {
			static CullFace cullFace = CullFace::Back;
			int chosen_index = (int)cullFace;
			if (ImGui::BeginCombo("Cull Face", CullFaceNames[chosen_index], ImGuiComboFlags_None)) {
				for (int ix = 0; ix < IM_ARRAYSIZE(CullFaceNames); ix++) {
					const bool is_selected = (chosen_index == ix);
					if (ImGui::Selectable(CullFaceNames[ix], is_selected)) {
						cullFace = (CullFace)ix;
						GraphicsAPI::Get()->SetCullFace(cullFace);
					}
					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
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
	if (ImGui::CollapsingHeader("FPS", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PlotLines("", frameRates.data(), (int)frameRates.size());
		const auto [minIt, maxIt] = std::minmax_element(frameRates.begin(), frameRates.end());
		ImGui::Text("[%.1f %.1f]", *minIt, *maxIt);
		static bool isVSync = false;
		if (ImGui::Checkbox("VSync", &isVSync)) { GraphicsContext::Get()->SetVSync(isVSync); }
	}

	ImGui::Separator();
	static bool shouldShowDemo = false;
	ImGui::Checkbox("Show Demo Window", &shouldShowDemo);
	ImGui::End();
	if (shouldShowDemo) ImGui::ShowDemoWindow();
	ImGui::Text("mainViewportSize: (%.1f, %.1f)", viewport->Size.x, viewport->Size.y);
}

