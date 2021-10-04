#include "ViewportPanel.h"

#include "Core/Math.h"
#include "Core/Input.h"
#include "Scene/Components.h"

#include <glad/glad.h> // include until Framebuffer and Texture abstractions are completed
#include <glm/gtc/type_ptr.hpp>

void ViewportPanel::OnImGuiRender() {
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

	if (selectedObject && gizmoShouldShow) {
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewportPanelAvailRegion.x, viewportPanelAvailRegion.y);

		auto& tc = selectedObject.get<TransformComponent>();
		auto transformMatrix = Math::ComposeTransform(tc.translation, tc.rotation, tc.scale);

		// Snapping
		const bool shouldSnap = Input::Get()->IsKeyPressed(340);
		// 45 degrees for rotation, 0.5m for translate and scale
		float snapValue = gizmoType == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f;
		float snapValues[3] = { snapValue, snapValue, snapValue };

		const glm::mat4& view = camera->GetViewMatrix();
		const glm::mat4& projection = camera->GetProjection();
		ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
			gizmoType, gizmoMode, glm::value_ptr(transformMatrix), nullptr, shouldSnap ? snapValues : nullptr);

		if (ImGuizmo::IsUsing()) {
			glm::vec3 translation, rotation, scale;
			Math::DecomposeTransform(transformMatrix, translation, rotation, scale);
			tc.translation = translation;
			// To prevent gimble-lock
			glm::vec3 deltaRotation = rotation - tc.rotation;
			tc.rotation += deltaRotation;
			tc.scale = scale;
		}
	}
	viewportPanelAvailRegionPrev = viewportPanelAvailRegion;
	ImGui::End();
	ImGui::PopStyleVar();
}

void ViewportPanel::OnEvent(Event& ev) {
	auto dispatcher = EventDispatcher(ev);
	dispatcher.Dispatch<MouseScrolledEvent>(AL_BIND_EVENT_FN(ViewportPanel::OnMouseScrolled));
	dispatcher.Dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(ViewportPanel::OnKeyPressed));
}

void ViewportPanel::OnMouseScrolled(MouseScrolledEvent& ev) {
	if (isViewportPanelHovered) camera->OnMouseScroll(ev.GetXOffset(), ev.GetYOffset());
}

void ViewportPanel::OnKeyPressed(KeyPressedEvent& ev) {
	Log::Debug("Key: {}", ev.GetKeyCode());
	switch (ev.GetKeyCode()) {
		// Transform Gizmos
	case 90: // Z
		gizmoShouldShow = true;
		gizmoType = ImGuizmo::OPERATION::TRANSLATE;
		break;
	case 88: // X
		gizmoShouldShow = true;
		gizmoType = ImGuizmo::OPERATION::ROTATE;
		break;
	case 67: // C
		gizmoShouldShow = true;
		gizmoType = ImGuizmo::OPERATION::SCALE;
		break;
	case 86: // V
		gizmoShouldShow = false;
		break;
	case 77: // M
		gizmoMode = (ImGuizmo::MODE)( (gizmoMode + 1) % 2 );
		break;
	}
}