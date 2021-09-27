#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/GraphicsAPI.h"
#include "Renderer/Shader.h"
#include "Renderer/FrameBuffer.h"
#include "Renderer/EditorCamera.h"
#include "Modeling/Modeling.h"

#include <glad/glad.h> // include until Framebuffer and Texture abstractions are completed
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <vector>


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
		fbo = FrameBuffer::Create(100, 100); // argument does not matter since it's going to be resized

		GraphicsAPI::Get()->Enable(GraphicsAbility::DepthTest);
		auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
		GraphicsAPI::Get()->SetClearColor(turquoise);
		camera = new EditorCamera(45, aspect, 0.01, 100);
	}

	virtual void OnUpdate(float ts) override {
		static float angle = 0.0f;
		static glm::mat4 model = glm::mat4(1.0f);
		std::shift_left(frameRates.begin(), frameRates.end(), 1);
		frameRates[frameRates.size() - 1] = 1.0f / ts;
		camera->OnUpdate(ts);

		//glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
		glm::mat4 projection = camera->GetProjection();
		//glm::vec3 eye({ 0.0, 0.0, 5.0 });
		//glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
		glm::mat4 view = camera->GetViewMatrix();
		if (false) {
			angle += ts * 0.5f;
			model = glm::rotate(model, ts, { 0, std::sin(angle), std::cos(angle) });
		}
		glm::mat4 mv = view * model;
		glm::mat4 mvp = projection * mv;
		glm::mat4 normalMatrix = glm::inverse(mv);

		GraphicsAPI::Get()->Clear();

		// Render into viewportFBO
		fbo->Bind();
		GraphicsAPI::Get()->SetClearColor({0, 0, 0, 1});
		GraphicsAPI::Get()->Clear();
		shader->Bind();
		shader->UploadUniformMat4("u_ModelViewPerspective", mvp); // needed for gl_Position;
		shader->UploadUniformInt("u_RenderType", 1); // Normal (2: UV)
		GraphicsAPI::Get()->DrawArrayTriangles(*vao);
		fbo->Unbind();
	}

	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}

	void OnKeyPressed(KeyPressedEvent& ev) {
		//Log::Debug("KeyPressed {}", ev.GetKeyCode());
	}

	virtual void OnDetach() override {
		delete fbo;
	}

	virtual void OnImGuiRender() override {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_None);

		ImGui::Begin("Left Panel");
		ImGui::Text("Scene Hierarchy will come here...");
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
			camera->SetViewportSize((int)viewportPanelAvailRegion.x, (int)viewportPanelAvailRegion.y);
		}
		ImGui::Image((void*)(intptr_t)fbo->GetColorAttachmentRendererID(0), ImVec2(viewportPanelAvailRegion.x, viewportPanelAvailRegion.y), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		aspect = viewportPanelAvailRegion.x / viewportPanelAvailRegion.y;
		ImGui::End();
		ImGui::PopStyleVar();

		static bool shouldShowDemo = false;

		ImGui::Begin("Right Panel"); {
			ImGui::Text("Components will come here");

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
	float aspect = 1.0f;
	EditorCamera* camera = nullptr;

	Shader* shader = nullptr;
	VertexArray* vao = nullptr;
	FrameBuffer* fbo = nullptr;

	std::vector<float> frameRates = std::vector<float>(120);
};