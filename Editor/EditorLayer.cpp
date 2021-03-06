#include "EditorLayer.h"

#include "ExampleScene.h"

#include "Core/GraphicsContext.h"
#include "Renderer/GraphicsAPI.h"
#include "Scene/Components.h"
#include "Renderer/Renderer.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

void EditorLayer::OnAttach() {
	// in case we'll see an area not behind any ImWindow
	auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
	GraphicsAPI::Get()->SetClearColor(turquoise);
	GraphicsAPI::Get()->Enable(GraphicsAbility::DepthTest);
	GraphicsAPI::Get()->Enable(GraphicsAbility::StencilTest);
	GraphicsAPI::Get()->Enable(GraphicsAbility::FaceCulling);
	// keep if stencil fails. replace if stencil pass, independent of depth.
	GraphicsAPI::Get()->SetStencilOperation(StencilAction::Keep, StencilAction::Replace, StencilAction::Replace);

	selectionShader = Shader::Create("assets/shaders/SelectionShader.glsl");
	shader = Shader::Create("assets/shaders/BasicShader.glsl");
	solidColorShader = Shader::Create("assets/shaders/SolidColor.glsl");
	outlineShader = Shader::Create("assets/shaders/Outline.glsl");
	viewUbo = UniformBuffer::Create("ViewData", sizeof(ViewData));
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
	ViewData viewData;
	viewData.projection = camera->GetProjection();
	viewData.view = camera->GetViewMatrix();
	const glm::vec3& camPos = camera->GetPosition();
	viewData.viewPosition = { camPos.x, camPos.y, camPos.z, 1.0f };
	viewUbo->UploadData((const void*)&viewData);

	// Lights System
	auto queryLights = scene.View<TransformComponent, LightComponent>();
	int ix = 0;
	Lights lightsData;
	for (const auto& [ent, transform, lightC] : queryLights.each()) {
		Light light;
		light.type = (int)lightC.type;
		light.color = { lightC.color.x, lightC.color.y, lightC.color.z, 0.0f };
		light.intensity = lightC.intensity;
		light.pointParams.position = { transform.translation.x, transform.translation.y, transform.translation.z, 1.0f };
		light.pointParams.attenuation = { lightC.pointParams.attenuation.x, lightC.pointParams.attenuation.y, lightC.pointParams.attenuation.z, 0.0f };
		light.directionalParams.direction = { lightC.directionalParams.direction.x, lightC.directionalParams.direction.y, lightC.directionalParams.direction.z, 0.0f };
		lightsData.lights[ix] = light;
		ix++;
		if (ix == MAX_LIGHTS) break;
	}
	lightsData.numLights = ix;
	lightsData.ambientLight = scene.ambientColor;
	lightsUbo->UploadData((const void*)&lightsData);

	GraphicsAPI::Get()->Clear();

	// Render pass 1: render scene into viewportFBO
	viewportFbo->Bind();
	GraphicsAPI::Get()->SetClearColor(scene.backgroundColor);
	GraphicsAPI::Get()->Clear();
	GraphicsAPI::Get()->SetStencilFunction(BufferTestFunction::Always, 1, 0xFF);
	unsigned int mask = 0x00;
	shader->Bind();
	auto query = scene.View<TransformComponent, MeshComponent, MeshRendererComponent>();
	for (const auto& [ent, transform, mesh, meshRenderer] : query.each()) {
		mask = (selectedObject && selectedObject.entity() == ent) ? 0xFF : 0x00;
		GraphicsAPI::Get()->SetStencilMask(mask);
		Renderer::RenderMesh(shader, viewData, transform, mesh, meshRenderer);
	}
	auto query2 = scene.View<TransformComponent, ProceduralMeshComponent, MeshRendererComponent>();
	for (const auto& [ent, transform, pMesh, meshRenderer] : query2.each()) {
		mask = (selectedObject && selectedObject.entity() == ent) ? 0xFF : 0x00;
		GraphicsAPI::Get()->SetStencilMask(mask);
		Renderer::RenderProceduralMesh(shader, viewData, transform, pMesh, meshRenderer);
	}
	shader->Unbind();

	// Overlay wireframe of hovered object, if any
	solidColorShader->Bind();
	GraphicsAPI::Get()->SetStencilMask(0x00);
	GraphicsAPI::Get()->Enable(GraphicsAbility::PolygonOffsetLine);
	GraphicsAPI::Get()->SetPolygonOffset(-1.0f, -1.0f); // prevent z-fighting btw the object and its wireframe
	GraphicsAPI::Get()->SetPolygonMode(PolygonMode::Line);
	if (hoveredObject) {
		const EntityHandle& obj = hoveredObject;
		solidColorShader->UploadUniformFloat4("u_Color", { 0.8f, 0.8f, 0.8f, 1.0f });
		if (obj.any_of<MeshComponent>()) {
			Renderer::RenderMesh(solidColorShader, viewData, obj.get<TransformComponent>(), obj.get<MeshComponent>(), obj.get<MeshRendererComponent>());
		}
		else if (obj.any_of<ProceduralMeshComponent>()) {
			Renderer::RenderProceduralMesh(solidColorShader, viewData, obj.get<TransformComponent>(), obj.get<ProceduralMeshComponent>(), obj.get<MeshRendererComponent>());
		}
	};
	GraphicsAPI::Get()->SetPolygonMode(PolygonMode::Fill);
	GraphicsAPI::Get()->Disable(GraphicsAbility::PolygonOffsetLine);
	solidColorShader->Unbind();

	// Outline selected object, if any
	GraphicsAPI::Get()->SetStencilFunction(BufferTestFunction::NotEqual, 1, 0xFF);
	GraphicsAPI::Get()->SetStencilMask(0x00);
	GraphicsAPI::Get()->Disable(GraphicsAbility::DepthTest);
	outlineShader->Bind();
	if (selectedObject) {
		const EntityHandle& obj = selectedObject;
		outlineShader->UploadUniformFloat4("u_Color", { 1.0f, 1.0f, 0.0f, 1.0f });
		if (obj.any_of<MeshComponent>()) {
			Renderer::RenderMesh(outlineShader, viewData, obj.get<TransformComponent>(), obj.get<MeshComponent>(), obj.get<MeshRendererComponent>());
		}
		else if (obj.any_of<ProceduralMeshComponent>()) {
			Renderer::RenderProceduralMesh(outlineShader, viewData, obj.get<TransformComponent>(), obj.get<ProceduralMeshComponent>(), obj.get<MeshRendererComponent>());
		}
	}
	GraphicsAPI::Get()->SetStencilMask(0xFF);
	GraphicsAPI::Get()->SetStencilFunction(BufferTestFunction::Always, 1, 0xFF);
	GraphicsAPI::Get()->Enable(GraphicsAbility::DepthTest);
	outlineShader->Unbind();
	viewportFbo->Unbind();

	// Render pass 2: render entityID of each object into an integer buffer
	selectionFbo->Bind();
	selectionFbo->Clear(-1); // value when not hovering on any object
	selectionShader->Bind();
	for (const auto& [ent, transform, mesh, meshRenderer] : query.each()) {
		Renderer::RenderVertexArrayEntityID(ent, selectionShader, viewData, transform, mesh.vao, meshRenderer);
	}
	for (const auto& [ent, transform, pMesh, meshRenderer] : query2.each()) {
		Renderer::RenderVertexArrayEntityID(ent, selectionShader, viewData, transform, pMesh.vao, meshRenderer);
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

		ImGui::ColorEdit3("Ambient Light", glm::value_ptr(scene.ambientColor));
		ImGui::ColorEdit4("Background Color", glm::value_ptr(scene.backgroundColor));

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
