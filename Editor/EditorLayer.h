#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"
#include "Renderer/GraphicsAPI.h"
#include "Renderer/Shader.h"
#include "Renderer/FrameBuffer.h"
#include "Renderer/EditorCamera.h"
#include "Modeling/Modeling.h"
#include "Scene/Scene.h"
#include "Scene/Components.h"

#include <glad/glad.h> // include until Framebuffer and Texture abstractions are completed
#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <cereal/cereal.hpp>

#include <algorithm>
#include <string>
#include <vector>

glm::mat4 GetTransformMatrix(TransformComponent tc) {
	glm::mat4 rotation = glm::toMat4(glm::quat(tc.rotation));
	return glm::translate(glm::mat4(1.0f), tc.translation)
		* rotation
		* glm::scale(glm::mat4(1.0f), tc.scale);
}

class EditorLayer : public Layer {
public:
	EditorLayer() : Layer("Editor Layer") { }

	virtual void OnAttach() override {
		// in case we'll see an area not behind any ImWindow
		GraphicsAPI::Get()->Enable(GraphicsAbility::DepthTest);
		auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
		GraphicsAPI::Get()->SetClearColor(turquoise);

		shader = Shader::Create("assets/shaders/BasicShader.glsl");
		fbo = FrameBuffer::Create(100, 100); // arguments does not matter since FBO's going to be resized

		camera = new EditorCamera(45, aspect, 0.01f, 100);

		// Hard-coded example scene
		using ObjectData = struct { 
			std::string name; 
			glm::vec3 pos; 
			std::string meshFilePath; 
			MeshRendererComponent::Visualization viz;
		};
		std::vector<ObjectData> sceneData = {
			{ "monkey1", { 0.75, 0.5, 0.0 }, "assets/models/suzanne_smooth.obj", MeshRendererComponent::Visualization::Normal, },
			{ "monkey2", { -0.5, -0.1, 0.0 }, "assets/models/suzanne.obj", MeshRendererComponent::Visualization::UV, },
			{ "torus", { 0.1, -0.4, 0.7 }, "assets/models/torus_smooth.obj", MeshRendererComponent::Visualization::Depth, },
		};
		for (ObjectData& obj : sceneData) {
			auto ent = scene.CreateEntity(obj.name);
			auto& transform = ent.get<TransformComponent>();
			transform.translation = obj.pos;
			transform.rotation = obj.pos;
			transform.scale = { 0.4, 0.4, 0.4 };
			ent.emplace<MeshComponent>(obj.meshFilePath);
			ent.emplace<MeshRendererComponent>(obj.viz);
		}
	}

	virtual void OnUpdate(float ts) override {
		std::shift_left(frameRates.begin(), frameRates.end(), 1);
		frameRates[frameRates.size() - 1] = 1.0f / ts;

		camera->OnUpdate(ts);
		glm::mat4 projection = camera->GetProjection();
		glm::mat4 view = camera->GetViewMatrix();

		GraphicsAPI::Get()->Clear();

		fbo->Bind(); // Render into viewportFBO
		GraphicsAPI::Get()->SetClearColor({0, 0, 0, 1});
		GraphicsAPI::Get()->Clear();
		shader->Bind();
		auto query = scene.View<TransformComponent, MeshComponent, MeshRendererComponent>();
		for (const auto& [ent, transform, mesh, meshRenderer] : query.each()) {
			glm::vec3& translation = transform.translation;
			glm::mat4 model = GetTransformMatrix(transform);
			glm::mat4 modelView = view * model;
			glm::mat4 modelViewProjection = projection * modelView;
			glm::mat4 normalMatrix = glm::inverse(modelView);
			shader->UploadUniformMat4("u_ModelViewPerspective", modelViewProjection);
			shader->UploadUniformMat4("u_View", view);

			shader->UploadUniformInt("u_RenderType", (int)meshRenderer.visualization);
			shader->UploadUniformFloat4("u_SolidColor", meshRenderer.solidColor);
			shader->UploadUniformFloat("u_DepthMax", meshRenderer.depthParams.max);
			shader->UploadUniformFloat("u_DepthPow", meshRenderer.depthParams.pow);

			VertexArray* vao = mesh.vao;
			if (vao == nullptr) { continue; }
			GraphicsAPI::Get()->DrawArrayTriangles(*vao);
		}
		fbo->Unbind();
	}

	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<MouseScrolledEvent>(AL_BIND_EVENT_FN(EditorLayer::OnMouseScrolled));
	}

	void OnMouseScrolled(MouseScrolledEvent& ev) {
		if(isViewportPanelHovered) camera->OnMouseScroll(ev.GetXOffset(), ev.GetYOffset());
	}

	virtual void OnDetach() override {
		delete fbo;
	}

	virtual void OnImGuiRender() override {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_None);

		// LEFT COLUMN: Scene Hierarchy Panel
		ImGui::Begin("Hierarchy");
		// List object names for selection
		auto query = scene.View<TagComponent>();
		for (const auto& [ent, tag] : query.each()) {
			if (ImGui::Selectable(tag.tag.c_str(), selectedObject == ent)) {
				selectedObject = scene.GetHandle(ent);
			}
		}
		if (ImGui::Button("Save")) { scene.Save(); }
		ImGui::SameLine();
		if (ImGui::Button("Load")) { scene.Load(); }
		// Deselect when clicking on an empty area
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) selectedObject = {};
		ImGui::End();

		// MIDDLE COLUMN: Viewport Panel displays content of viewportFBO
		static ImVec2 viewportPanelAvailRegionPrev;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground); // ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
		ImVec2 viewportPanelAvailRegion = ImGui::GetContentRegionAvail();
		bool isViewportPanelResized = viewportPanelAvailRegion.x != viewportPanelAvailRegionPrev.x || viewportPanelAvailRegion.y != viewportPanelAvailRegionPrev.y;
		if (isViewportPanelResized) {
			fbo->Resize((int)viewportPanelAvailRegion.x, (int)viewportPanelAvailRegion.y);
			glViewport(0, 0, (int)viewportPanelAvailRegion.x, (int)viewportPanelAvailRegion.y);
			camera->SetViewportSize(viewportPanelAvailRegion.x, viewportPanelAvailRegion.y);
		}
		ImGui::Image((void*)(intptr_t)fbo->GetColorAttachmentRendererID(0), ImVec2(viewportPanelAvailRegion.x, viewportPanelAvailRegion.y), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		aspect = viewportPanelAvailRegion.x / viewportPanelAvailRegion.y;
		isViewportPanelHovered = ImGui::IsWindowHovered();
		ImGui::End();
		ImGui::PopStyleVar();

		// RIGHT COLUMN: Inspector Panel
		static bool shouldShowDemo = false;
		ImGui::Begin("Inspector"); 
		ImGui::Text("Components");
		if (selectedObject) {
			ImGui::InputText("Tag", &selectedObject.get<TagComponent>().tag);
			scene.Visit(selectedObject, [&](const entt::type_info info) {
				if (info == entt::type_id<TransformComponent>()) {
					if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
						auto& transform = selectedObject.get<TransformComponent>();
						ImGui::InputFloat3("Translation", glm::value_ptr(transform.translation));
						ImGui::InputFloat3("Rotation", glm::value_ptr(transform.rotation));
						ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale));
					}					
				}
				else if (info == entt::type_id<MeshComponent>()) {
					if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
						auto& mesh = selectedObject.get<MeshComponent>();
						std::string& filepath = selectedObject.get<MeshComponent>().filepath;
							
						char buffer[256] = { 0 };
						strcpy_s(buffer, sizeof(buffer), filepath.c_str());
						if (ImGui::InputText("OBJ File", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
							mesh.filepath = std::string(buffer);
							mesh.LoadOBJ();
						}
					}
				}
				else if (info == entt::type_id<MeshRendererComponent>()) {
					if (ImGui::CollapsingHeader("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {
						auto& meshRenderer = selectedObject.get<MeshRendererComponent>();
						int chosen_index = (int)meshRenderer.visualization;
						if (ImGui::BeginCombo("Visualizations", MeshRendererComponent::visNames[chosen_index], ImGuiComboFlags_None)) {
							for (int ix = 0; ix < IM_ARRAYSIZE(MeshRendererComponent::visNames); ix++) {
								const bool is_selected = (chosen_index == ix);
								if (ImGui::Selectable(MeshRendererComponent::visNames[ix], is_selected)) {
									meshRenderer.visualization = (MeshRendererComponent::Visualization)ix;
								}
								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_selected) ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}
						switch (meshRenderer.visualization) {
						case MeshRendererComponent::Visualization::Depth:
							ImGui::SliderFloat("MaxDepth", &meshRenderer.depthParams.max, 0.01f, 100.0f);
							ImGui::SliderFloat("Pow (Contrast)", &meshRenderer.depthParams.pow, 0.25f, 4.0f);
							break;
						case MeshRendererComponent::Visualization::SolidColor:
							ImGui::ColorEdit4("Solid Color", glm::value_ptr(meshRenderer.solidColor));
							break;
						}
					}
				}
			});
		}

		ImGui::Separator();
		ImGui::Text("Stats:\n"
			"mainViewportSize: (%.1f, %.1f)\n"
			"viewportPanelAvailRegion: (%.1f, %.1f)\n"
			"",
			viewport->Size.x, viewport->Size.y,
			viewportPanelAvailRegion.x, viewportPanelAvailRegion.y
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
		ImGui::Checkbox("Show Demo Window", &shouldShowDemo);
		viewportPanelAvailRegionPrev = viewportPanelAvailRegion;
		ImGui::End();

		if (shouldShowDemo) ImGui::ShowDemoWindow();
	}

private:
	Shader* shader = nullptr;
	FrameBuffer* fbo = nullptr;

	float aspect = 1.0f;
	EditorCamera* camera = nullptr;
	bool isViewportPanelHovered = false;
	std::vector<float> frameRates = std::vector<float>(120);

	Scene scene;
	EntityHandle selectedObject = {};
};