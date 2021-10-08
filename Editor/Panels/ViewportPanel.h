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
	ViewportPanel(FrameBuffer*& viewportFbo, FrameBuffer*& selectionFbo, int& hoveredEntityId, EditorCamera*& camera, EntityHandle& selectedObject, EntityHandle& hoveredObject, int& mouseX, int& mouseY)
		: viewportFbo(viewportFbo), selectionFbo(selectionFbo), hoveredEntityId(hoveredEntityId), camera(camera), selectedObject(selectedObject), hoveredObject(hoveredObject), mouseX(mouseX), mouseY(mouseY) {}

	void OnImGuiRender();
	void OnEvent(Event& ev);
	void OnMouseScrolled(MouseScrolledEvent& ev);
	void OnMouseClicked (MouseButtonPressedEvent& ev);
	void OnKeyPressed(KeyPressedEvent& ev);

private:
	// references to EditorLayer's members
	FrameBuffer*& viewportFbo;
	FrameBuffer*& selectionFbo;
	int& mouseX;
	int& mouseY;
	int& hoveredEntityId;
	EditorCamera*& camera;
	EntityHandle& selectedObject;
	EntityHandle& hoveredObject;

	bool isViewportPanelHovered = false;
	float aspect = 1.0f;
	bool gizmoShouldShow = false;
	ImGuizmo::OPERATION gizmoType = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;
};