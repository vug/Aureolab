#include "InspectorPanel.h"

#include "Scene/Components.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>
#include <glm/gtc/type_ptr.hpp>

void InspectorPanel::OnImGuiRender() {
	ImGui::Begin("Inspector");
	ImGui::Text("Components");
	if (selectedObject) {
		ImGui::InputText("Tag", &selectedObject.get<TagComponent>().tag);

		// List the components of the selected object
		scene.Visit(selectedObject, [&](const entt::type_info info) {
			if (info == entt::type_id<TransformComponent>()) {
				if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
					auto& transform = selectedObject.get<TransformComponent>();
					ImGui::InputFloat3("Translation", glm::value_ptr(transform.translation));
					ImGui::InputFloat3("Rotation", glm::value_ptr(transform.rotation));
					ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale));
				}
			}
			else if (info == entt::type_id<MeshComponent>()) {
				if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
					auto& mesh = selectedObject.get<MeshComponent>();
					std::string& filepath = selectedObject.get<MeshComponent>().filepath;

					char buffer[256] = { 0 };
					strcpy_s(buffer, sizeof(buffer), filepath.c_str());
					if (ImGui::InputText("OBJ File", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
						mesh.filepath = std::string(buffer);
						mesh.LoadOBJ();
					}
				}
			}
			else if (info == entt::type_id<MeshRendererComponent>()) {
				if (ImGui::CollapsingHeader("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen)) {
					auto& meshRenderer = selectedObject.get<MeshRendererComponent>();
					int chosen_index = (int)meshRenderer.visualization;
					if (ImGui::BeginCombo("Visualizations", MeshRendererComponent::visNames[chosen_index], ImGuiComboFlags_None)) {
						for (int ix = 0; ix < IM_ARRAYSIZE(MeshRendererComponent::visNames); ix++) {
							const bool is_selected = (chosen_index == ix);
							if (ImGui::Selectable(MeshRendererComponent::visNames[ix], is_selected)) {
								meshRenderer.visualization = (MeshRendererComponent::Visualization)ix;
							}
							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected) ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}
					switch (meshRenderer.visualization) {
					case MeshRendererComponent::Visualization::Depth:
						ImGui::SliderFloat("MaxDepth", &meshRenderer.depthParams.max, 0.01f, 100.0f);
						ImGui::SliderFloat("Pow (Contrast)", &meshRenderer.depthParams.pow, 0.25f, 4.0f);
						break;
					case MeshRendererComponent::Visualization::SolidColor:
						ImGui::ColorEdit4("Solid Color", glm::value_ptr(meshRenderer.solidColor));
						break;
					}
				}
			}
		});

		if (ImGui::Button("Add Component")) { ImGui::OpenPopup("AddComponent"); }
		if (ImGui::BeginPopup("AddComponent")) {
			if (!selectedObject.any_of<MeshComponent>() && ImGui::MenuItem("Mesh Component")) {
				selectedObject.emplace<MeshComponent>();
			}
			if (!selectedObject.any_of<MeshRendererComponent>() &&  ImGui::MenuItem("Mesh Renderer Component")) {
				selectedObject.emplace<MeshRendererComponent>();
			}
			ImGui::EndPopup();
		}
	}

	ImGui::End();
}