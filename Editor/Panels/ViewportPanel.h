#pragma once

#include "Events/Event.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer/FrameBuffer.h"
#include "Renderer/EditorCamera.h"
#include "Scene/Scene.h"

#include <imgui.h>
#include <ImGuizmo.h>

// Displays content of viewportFBO and transform gizmos
class ViewportPanel {
public:
	ViewportPanel() = default;
	ViewportPanel(FrameBuffer*& fbo, EditorCamera*& camera, EntityHandle& selectedObject)
		: fbo(fbo), camera(camera), selectedObject(selectedObject) {}

	void OnImGuiRender();
	void OnEvent(Event& ev);
	void OnMouseScrolled(MouseScrolledEvent& ev);
	void OnKeyPressed(KeyPressedEvent& ev);

private:
	// references to EditorLayer's members
	FrameBuffer*& fbo;
	EditorCamera*& camera;
	EntityHandle& selectedObject;

	bool isViewportPanelHovered = false;
	float aspect = 1.0f;
	bool gizmoShouldShow = false;
	ImGuizmo::OPERATION gizmoType = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;
};