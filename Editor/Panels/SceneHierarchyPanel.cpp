#include "SceneHierarchyPanel.h"

#include "Scene/Components.h"

#include <imgui.h>

void SceneHierarchyPanel::OnImGuiRender() {
	ImGui::Begin("Hierarchy");
	// List object names for selection
	auto query = scene.View<TagComponent>();
	for (const auto& [ent, tag] : query.each()) {
		if (ImGui::Selectable(tag.tag.c_str(), selectedObject == ent)) {
			selectedObject = scene.GetHandle(ent);
		}
	}

	// Deselect when clicking on an empty area
	if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) selectedObject = {};
	ImGui::End();

	// Right-click on a blank space
	if (ImGui::BeginPopupContextWindow("testing", ImGuiMouseButton_Right, false)) {
		if (ImGui::MenuItem("Create Empty Entity")) { scene.CreateEntity("Unnamed Entity"); }
		ImGui::EndPopup();
	}
}