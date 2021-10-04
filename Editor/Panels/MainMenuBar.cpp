#include "MainMenuBar.h"

#include "Platform/Platform.h"

#include <imgui.h>

void MainMenuBar::OnImGuiRender() {
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New")) {
			scene.New();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Save As")) {
			std::string filepath = PlatformUtils::SaveFile("AureoLab Scene (*.scene)\0*.scene\0");
			if (!filepath.empty()) scene.SaveToFile(filepath);
		}
		if (ImGui::MenuItem("Load")) {
			std::string filepath = PlatformUtils::OpenFile("AureoLab Scene (*.scene)\0*.scene\0");
			if (!filepath.empty()) scene.LoadFromFile(filepath);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Quit")) {
			exit(0);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Scene")) {
		if (ImGui::MenuItem("Create Empty Entity")) { scene.CreateEntity("Unnamed Entity"); }
		ImGui::Separator();
		if (ImGui::MenuItem("Take Snapshot")) { scene.SaveToMemory(); }
		if (ImGui::MenuItem("Load Snapshot")) { scene.LoadFromMemory(); }
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}
