#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/GraphicsAPI.h"

#include "Renderer/Shader.h"
#include "Renderer/FrameBuffer.h"
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
		std::vector<BasicVertex> vertices = LoadOBJ("assets/models/torus_smooth.obj");
		VertexBuffer* vbo = VertexBuffer::Create(BasicVertexAttributeSpecs);
		vbo->SetVertices(vertices);
		Log::Debug("num vertices: {}", vertices.size());
		vao = VertexArray::Create();
		vao->AddVertexBuffer(*vbo);
		fbo = FrameBuffer::Create();

		ga = GraphicsAPI::Create();
		ga->Initialize();
		ga->Enable(GraphicsAbility::DepthTest);
		auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
		ga->SetClearColor(turquoise);

		fbo->Bind();
		// generate color buffer texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glBindTexture(GL_TEXTURE_2D, 0);
		// generate depth buffer texture
		glGenTextures(1, &depth_texture);
		glBindTexture(GL_TEXTURE_2D, depth_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 100, 100, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		// attach them to currently bound framebuffer object
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
		// Check FBO completion
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
			Log::Debug("Framebuffer complete.");
		}
		else {
			Log::Warning("Framebuffer incomplete.");
		}
		fbo->Unbind();

	}

	virtual void OnUpdate(float ts) override {
		static float time = 0.0f;
		static float angle = 0.0f;
		static glm::mat4 model = glm::mat4(1.0f);
		std::shift_left(frameRates.begin(), frameRates.end(), 1);
		frameRates[frameRates.size() - 1] = 1.0f / ts;		

		float red = std::sin(time);
		time += ts;

		glm::mat4 projection = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.0f);
		glm::vec3 eye({ 0.0, 0.0, 5.0 });
		glm::mat4 view = glm::lookAt(eye, glm::vec3{ 0.0, 0.0, 0.0 }, glm::vec3{ 0.0, 1.0, 0.0 });
		if (true) {
			angle += ts * 0.5f;
			model = glm::rotate(model, ts, { 0, std::sin(angle), std::cos(angle) });
		}
		glm::mat4 mv = view * model;
		glm::mat4 mvp = projection * mv;
		glm::mat4 normalMatrix = glm::inverse(mv);

		ga->Clear();

		// Render into viewportFBO
		fbo->Bind();
		ga->SetClearColor({0, 0, 0, 1});
		ga->Clear();
		shader->Bind();
		shader->UploadUniformMat4("u_ModelViewPerspective", mvp); // needed for gl_Position;
		shader->UploadUniformInt("u_RenderType", 1); // Normal (2: UV)
		ga->DrawArrayTriangles(*vao);
		fbo->Unbind();
	}

	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		//dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(EditorLayer::OnWindowResize));
	}

	virtual void OnDetach() override {
		delete ga;
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
		ImVec2 viewportPanelSize = ImGui::GetWindowSize();
		ImVec2 viewportPanelPos = ImGui::GetWindowPos();
		ImVec2 viewportPanelAvailRegion = ImGui::GetContentRegionAvail();
		bool isViewportPanelResized = viewportPanelAvailRegion.x != viewportPanelAvailRegionPrev.x || viewportPanelAvailRegion.y != viewportPanelAvailRegionPrev.y;
		if (isViewportPanelResized) {
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportPanelAvailRegion.x, viewportPanelAvailRegion.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindTexture(GL_TEXTURE_2D, depth_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewportPanelAvailRegion.x, viewportPanelAvailRegion.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
			glViewport(0, 0, viewportPanelAvailRegion.x, viewportPanelAvailRegion.y);
		}
		ImGui::Image((void*)(intptr_t)texture, ImVec2(viewportPanelAvailRegion.x, viewportPanelAvailRegion.y));
		aspect = viewportPanelAvailRegion.x / viewportPanelAvailRegion.y;
		ImGui::End();
		ImGui::PopStyleVar();

		static bool shouldShowDemo = false;

		ImGui::Begin("Right Panel"); {
			ImGui::Text("Components will come here");
			int glViewportData[4];
			glGetIntegerv(GL_VIEWPORT, glViewportData);
			ImGui::Text("Stats:\n"
				"mainViewportSize: (%.1f, %.1f)\n"
				"mainViewportWorkSize: (%.1f, %.1f)\n"
				"GL_VIEWPORT: (%d, %d)\n"
				"--\n"
				"viewportPanelPos: (%.1f, %.1f)\n"
				"viewportPanelSize: (%.1f, %.1f)\n"
				"viewportPanelAvailRegion: (%.1f, %.1f)\n"
				"",
				viewport->Size.x, viewport->Size.y,
				viewport->WorkSize.x, viewport->WorkSize.y,
				glViewportData[2], glViewportData[3],
				viewportPanelPos.x, viewportPanelPos.y,
				viewportPanelSize.x, viewportPanelSize.y,
				viewportPanelAvailRegion.x, viewportPanelAvailRegion.y
			);

			ImGui::Separator();
			ImGui::PlotLines("FPS", frameRates.data(), frameRates.size());
			const auto [minIt, maxIt] = std::minmax_element(frameRates.begin(), frameRates.end());
			ImGui::Text("[%.1f %.1f]", *minIt, *maxIt);

			ImGui::Separator();
			ImGui::Checkbox("Show Demo Window", &shouldShowDemo);

			viewportPanelAvailRegionPrev = viewportPanelAvailRegion;
		} ImGui::End();

		if (shouldShowDemo) ImGui::ShowDemoWindow();
	}

private:
	GraphicsAPI* ga = nullptr;
	float aspect = 1.0f;

	Shader* shader = nullptr;
	VertexArray* vao = nullptr;
	FrameBuffer* fbo = nullptr;
	unsigned int fboID = 0;
	unsigned int texture = 0;
	unsigned int depth_texture = 0;

	std::vector<float> frameRates = std::vector<float>(120);
};