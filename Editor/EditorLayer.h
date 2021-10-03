#pragma once

#include "Panels/SceneHierarchyPanel.h"

#include "Core/Layer.h"
#include "Core/Math.h"
#include "Core/Input.h"
#include "Core/GraphicsContext.h"
#include "Events/Event.h"
#include "Events/KeyEvent.h"
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
#include <ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <cereal/cereal.hpp>

#include <algorithm>
#include <string>
#include <vector>

class EditorLayer : public Layer {
public:
	EditorLayer() : Layer("Editor Layer") { }

	virtual void OnAttach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnDetach() override;

	virtual void OnEvent(Event& ev) override;
	virtual void OnImGuiRender() override;

	void OnMouseScrolled(MouseScrolledEvent& ev); 
	void OnKeyPressed(KeyPressedEvent& ev);

private:
	Shader* shader = nullptr;
	FrameBuffer* fbo = nullptr;

	EditorCamera* camera = nullptr;
	float aspect = 1.0f;
	glm::mat4 projection = glm::mat4{ 1.0f };
	glm::mat4 view = glm::mat4{ 1.0f };
	bool isViewportPanelHovered = false;

	bool gizmoShouldShow = false;
	ImGuizmo::OPERATION gizmoType = ImGuizmo::OPERATION::TRANSLATE;

	std::vector<float> frameRates = std::vector<float>(120);

	Scene scene;
	EntityHandle selectedObject = {};

	SceneHierarchyPanel hierarchyPanel{ &scene, &selectedObject };
};