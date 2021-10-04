#include "SceneHierarchyPanel.h"

#include "Scene/Components.h"

#include <imgui.h>

void SceneHierarchyPanel::OnImGuiRender() {
	ImGui::Begin("Hierarchy");
	// List object names for selection and deletion
	auto query = scene.View<TagComponent>();
	for (const auto& [ent, tag] : query.each()) {
		EntityHandle entHandle = scene.GetHandle(ent);
		if (ImGui::Selectable(tag.tag.c_str(), selectedObject == ent)) {
			selectedObject = entHandle;
		}

		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Object")) {
				if (selectedObject == entHandle) { selectedObject = {}; }
				scene.DestroyEntity(entHandle);
			}
			ImGui::EndPopup();
		}
	}

	// Right-click on a blank space
	if (ImGui::BeginPopupContextWindow("testing", ImGuiMouseButton_Right, false)) {
		if (ImGui::MenuItem("Create Empty Entity")) { scene.CreateEntity("Unnamed Entity"); }
		ImGui::EndPopup();
	}

	// Deselect when clicking on an empty area
	if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) selectedObject = {};
	ImGui::End();
}