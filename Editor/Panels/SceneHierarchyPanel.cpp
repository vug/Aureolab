#include "SceneHierarchyPanel.h"

#include "Scene/Components.h"
#include "Platform/Platform.h"

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
	if (ImGui::Button("Save As")) {
		std::string filepath = PlatformUtils::SaveFile("AureoLab Scene (*.scene)\0*.scene\0");
		if (!filepath.empty()) scene.SaveToFile(filepath);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		std::string filepath = PlatformUtils::OpenFile("AureoLab Scene (*.scene)\0*.scene\0");
		if (!filepath.empty()) scene.LoadFromFile(filepath);
	}
	if (ImGui::Button("Take Snapshot")) { scene.SaveToMemory(); }
	ImGui::SameLine();
	if (ImGui::Button("Load Snapshot")) { scene.LoadFromMemory(); }

	// Deselect when clicking on an empty area
	if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) selectedObject = {};
	ImGui::End();
}