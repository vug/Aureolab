#pragma once

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/GraphicsAPI.h"

#include "Renderer/Shader.h"
#include "Modeling/Modeling.h"

#include <glad/glad.h> // include until Framebuffer and Texture abstractions are completed
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


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

		ga = GraphicsAPI::Create();
		ga->Initialize();
		ga->Enable(GraphicsAbility::DepthTest);
		ga->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });

		// generate framebuffer and activate
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		// generate color buffer texture
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 750, 750, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glBindTexture(GL_TEXTURE_2D, 0);
		// generate depth buffer texture
		unsigned int depth_texture = 0;
		glGenTextures(1, &depth_texture);
		glBindTexture(GL_TEXTURE_2D, depth_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 750, 750, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
		// attach them to currently bound framebuffer object
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
			Log::Debug("Framebuffer complete.");
		}
		else {
			Log::Warning("Framebuffer incomplete.");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	virtual void OnUpdate(float ts) override {
		static float time = 0.0f;
		static float angle = 0.0f;
		static glm::mat4 model = glm::mat4(1.0f);

		float red = std::sin(time);
		time += ts;

		float aspect = 1.0;
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

		auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
		ga->SetClearColor(turquoise);
		ga->Clear();

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		ga->SetClearColor({1, 0, 0, 1});
		ga->Clear();
		shader->Bind();
		shader->UploadUniformMat4("u_ModelViewPerspective", mvp); // needed for gl_Position;
		shader->UploadUniformInt("u_RenderType", 1); // Normal (2: UV)
		ga->DrawArrayTriangles(*vao);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Viewport ImWindow
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground); // ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
		ImGui::Image((void*)(intptr_t)texture, ImVec2(750, 750));
		ImGui::End();
		ImGui::PopStyleVar();
	}

	virtual void OnEvent(Event& ev) override {
		auto dispatcher = EventDispatcher(ev);
		dispatcher.Dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(EditorLayer::OnWindowResize));
	}

	void OnWindowResize(WindowResizeEvent& e) {
		Log::Debug("Layer2 received: {}", e.ToString());
		aspect = (float)e.GetWidth() / e.GetHeight();
	}

	virtual void OnDetach() override {
		delete ga;
		glDeleteFramebuffers(1, &fbo);
	}

	virtual void OnImGuiRender() override {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_None);

		//ImGui::SetNextWindowSize({ 500.0f, 100.0f }); // ImGuiCond_Always (default), ImGuiCond_FirstUseEver, 
		//ImGui::SetNextWindowPos({ 0.0f, 100.0f });
		//ImGui::SetNextWindowContentSize({ 50.0f, 50.0f });
		ImGui::Begin("Editor Layer");
		ImGui::Text("Laylay...");
		ImGui::End();

		static bool shouldShowDemo = false;

		//ImGuiViewport* viewport = ImGui::GetMainViewport();
		//ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_None);
		//ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_PassthruCentralNode);
		//ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_NoDockingInCentralNode);

		int glViewportData[4];
		glGetIntegerv(GL_VIEWPORT, glViewportData);

		ImGui::Begin("Stats"); {
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
			ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			ImVec2 viewportOffset = ImGui::GetWindowPos();
			ImVec2 m_ViewportBounds[2];
			m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
			m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

			ImGui::Text("Welcome to AureoLab Editor.");
			ImGui::Checkbox("Show Demo Window", &shouldShowDemo);
			ImGui::Text("Stats:\n"
				"viewportSize: (%.1f, %.1f)\n"
				"viewportWorkSize: (%.1f, %.1f)\n"
				"GL_VIEWPORT: (%d, %d)\n"
				"GetContentRegionAvail: (%.1f, %.1f)\n"
				"GetWindowContentRegionMin: (%.1f, %.1f)\n"
				"GetWindowContentRegionMax: (%.1f, %.1f)\n"
				"GetWindowPos: (%.1f, %.1f)\n"
				"m_ViewportBounds[0]: (%.1f, %.1f)\n"
				"m_ViewportBounds[1]: (%.1f, %.1f)\n"
				"m_ViewportBounds Size: (%.1f, %.1f)\n"
				"",
				viewport->Size.x, viewport->Size.y,
				viewport->WorkSize.x, viewport->WorkSize.y,
				glViewportData[2], glViewportData[3],
				viewportPanelSize.x, viewportPanelSize.y,
				viewportMinRegion.x, viewportMinRegion.y,
				viewportMaxRegion.x, viewportMaxRegion.y,
				viewportOffset.x, viewportOffset.y,
				m_ViewportBounds[0].x, m_ViewportBounds[0].y,
				m_ViewportBounds[1].x, m_ViewportBounds[1].y,
				m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y
			);
		} ImGui::End();

		if (shouldShowDemo) ImGui::ShowDemoWindow();
	}

private:
	GraphicsAPI* ga = nullptr;
	float aspect = 1.0f;

	Shader* shader = nullptr;
	VertexArray* vao = nullptr;
	unsigned int fbo = 0;
	unsigned int texture = 0;
};