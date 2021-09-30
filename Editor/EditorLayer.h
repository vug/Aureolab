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
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <cereal/cereal.hpp>

#include <algorithm>
#include <string>
#include <vector>

glm::mat4 GetTransformMatrix(TransformComponent tc) {
	glm::mat4 rotation = glm::toMat4(glm::quat(tc.Rotation));
	return glm::translate(glm::mat4(1.0f), tc.Translation)
		* rotation
		* glm::scale(glm::mat4(1.0f), tc.Scale);
}

class EditorLayer : public Layer {
public:
	EditorLayer() : Layer("Editor Layer") { }

	virtual void OnAttach() override {
		shader = Shader::Create("assets/shaders/BasicShader.glsl");

		std::vector<BasicVertex> vertices = LoadOBJ("assets/models/suzanne_smooth.obj");
		VertexBuffer* vbo = VertexBuffer::Create(BasicVertexAttributeSpecs);
		vbo->SetVertices(vertices);
		Log::Debug("num vertices: {}", vertices.size());
		vao = VertexArray::Create();
		vao->AddVertexBuffer(*vbo);

		fbo = FrameBuffer::Create(100, 100); // arguments does not matter since FBO's going to be resized

		GraphicsAPI::Get()->Enable(GraphicsAbility::DepthTest);
		auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
		GraphicsAPI::Get()->SetClearColor(turquoise);
		camera = new EditorCamera(45, aspect, 0.01f, 100);

		// Hard-coded example scene
		using ObjectData = struct { std::string name; glm::vec3 pos; };
		std::vector<ObjectData> sceneData = {
			{"monkey1", { 0.75, 0.5, 0.0 }},
			{ "monkey2", { -0.5, -0.1, 0.0 } },
			{ "monkey3", { 0.1, -0.4, 0.7 } },
		};
		for (ObjectData& obj : sceneData) {
			auto ent = scene.CreateEntity(obj.name);
			auto& transform = ent.get<TransformComponent>();
			transform.Translation = obj.pos;
			transform.Rotation = obj.pos;
			transform.Scale = { 0.4, 0.4, 0.4 };
		}
	}

	virtual void OnUpdate(float ts) override {
		static float angle = 0.0f;
		static glm::mat4 model = glm::mat4(1.0f);
		std::shift_left(frameRates.begin(), frameRates.end(), 1);
		frameRates[frameRates.size() - 1] = 1.0f / ts;
		camera->OnUpdate(ts);

		glm::mat4 projection = camera->GetProjection();
		glm::mat4 view = camera->GetViewMatrix();

		GraphicsAPI::Get()->Clear();

		// Render into viewportFBO
		fbo->Bind();
		GraphicsAPI::Get()->SetClearColor({0, 0, 0, 1});
		GraphicsAPI::Get()->Clear();
		shader->Bind();
		shader->UploadUniformInt("u_RenderType", 1); // Normal (2: UV)

		auto query = scene.View<TransformComponent>();
		for (const auto& [ent, transform] : query.each()) {
			glm::vec3& translation = transform.Translation;
			//Log::Debug("Ent [{}], Pos: ({}, {}, {})", ent, translation.x, translation.y, translation.z);
			glm::mat4 model = GetTransformMatrix(transform);
			glm::mat4 modelView = view * model;
			glm::mat4 modelViewProjection = projection * modelView;
			glm::mat4 normalMatrix = glm::inverse(modelView);
			shader->UploadUniformMat4("u_ModelViewPerspective", modelViewProjection);
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

		ImGui::Begin("Hierarchy");
		// List object names
		auto query = scene.View<TagComponent>();
		for (const auto& [ent, tag] : query.each()) {
			if (ImGui::Selectable(tag.Tag.c_str(), selectedObject == ent)) {
				selectedObject = scene.GetHandle(ent);
			}
		}

		// Deselect when clicking on an empty area
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) selectedObject = {};
		ImGui::End();

		// Viewport ImWindow displays content of viewportFBO
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

		static bool shouldShowDemo = false;

		ImGui::Begin("Inspector"); {
			ImGui::Text("Components");
			if (selectedObject) {
				scene.Visit(selectedObject, [&](const entt::type_info info) {
					if (info == entt::type_id<TransformComponent>()) {
						auto& transform = selectedObject.get<TransformComponent>();
						ImGui::InputFloat3("Translation", glm::value_ptr(transform.Translation));
						ImGui::InputFloat3("Rotation", glm::value_ptr(transform.Rotation));
						ImGui::InputFloat3("Scale", glm::value_ptr(transform.Scale));
					}
					else if (info == entt::type_id<TagComponent>()) {
						std::string& tag = selectedObject.get<TagComponent>().Tag;
						ImGui::InputText("TagComponent", (char*)tag.c_str(), tag.capacity() + 1);
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
				if (ImGui::Checkbox("VSync", &isVSync)) {
					GraphicsContext::Get()->SetVSync(isVSync);
				}
			}

			if (ImGui::CollapsingHeader("Editor Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("Yaw, Pitch, Roll: (%.2f, %.2f, %.2f)", camera->GetYaw(), camera->GetPitch(), camera->GetRoll());
				ImGui::Text("Pos: (%.1f, %.1f, %.1f)", camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
				ImGui::Text("Target: (%.1f, %.1f, %.1f)", camera->GetFocalPoint().x, camera->GetFocalPoint().y, camera->GetFocalPoint().z);
				ImGui::Text("Distance: %.1f", camera->GetDistance());
				float fov = camera->GetFOV();
				if (ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f)) {
					camera->SetFOV(fov);
				}
				ImGui::SliderFloat("Roll", camera->GetRefRoll(), 0.0f, 3.141593f);

			}

			ImGui::Separator();
			ImGui::Checkbox("Show Demo Window", &shouldShowDemo);
			viewportPanelAvailRegionPrev = viewportPanelAvailRegion;
		} ImGui::End();

		if (shouldShowDemo) ImGui::ShowDemoWindow();
	}

private:
	Shader* shader = nullptr;
	VertexArray* vao = nullptr;
	FrameBuffer* fbo = nullptr;

	float aspect = 1.0f;
	EditorCamera* camera = nullptr;
	bool isViewportPanelHovered = false;
	std::vector<float> frameRates = std::vector<float>(120);

	Scene scene;
	EntityHandle selectedObject = {};
};