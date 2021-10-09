#include "InspectorPanel.h"

#include "Scene/Components.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_internal.h> // for PushMultiItemsWidths, GImGui
#include <entt/entt.hpp>
#include <glm/gtc/type_ptr.hpp>

static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
	bool value_changed = false;
	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	// https://coolors.co/ff595e-ffca3a-8ac926-1982c4-6a4c93
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 1.0f, 0.34901961f, 0.36862745f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.000000f, 0.521569f, 0.537255f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 1.000000f, 0.200000f, 0.227451f, 1.0f });

	if (ImGui::Button("X", buttonSize)) { values.x = resetValue; value_changed = true; }
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	value_changed |= ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.541176f, 0.788235f, 0.149020f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.631373f, 0.858824f, 0.262745f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.462745f, 0.670588f, 0.129412f, 1.0f });
	if (ImGui::Button("Y", buttonSize)) { values.y = resetValue; value_changed = true; }
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	value_changed |= ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.098039f, 0.509804f, 0.768627f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.149020f, 0.607843f, 0.890196f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.082353f, 0.423529f, 0.635294f, 1.0f });
	if (ImGui::Button("Z", buttonSize)) { values.z = resetValue; value_changed = true; }
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	value_changed |= ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);
	ImGui::PopID();

	return value_changed;
}

void InspectorPanel::OnImGuiRender() {
	ImGui::Begin("Inspector");
	ImGui::Text("Hovering: %s", hoveredObject ? hoveredObject.get<TagComponent>().tag.c_str() : "");
	ImGui::Separator();
	ImGui::Text("Components");
	if (selectedObject) {
		ImGui::InputText("Tag", &selectedObject.get<TagComponent>().tag);

		// List the components of the selected object
		scene.Visit(selectedObject, [&](const entt::type_info info) {
			if (info == entt::type_id<TransformComponent>()) {
				if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
					auto& transform = selectedObject.get<TransformComponent>();
					DrawVec3Control("Translation", transform.translation);
					DrawVec3Control("Rotation", transform.rotation);
					DrawVec3Control("Scale", transform.scale, 1.0f);
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
				if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
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
			else if (info == entt::type_id<ProceduralMeshComponent>()) {
				if (ImGui::CollapsingHeader("Procedural Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
					auto& pMesh = selectedObject.get<ProceduralMeshComponent>();
					int chosen_index = (int)pMesh.parameters.shape;
					if (ImGui::BeginCombo("Shapes", ProceduralMeshComponent::shapeNames[chosen_index], ImGuiComboFlags_None)) {
						for (int ix = 0; ix < IM_ARRAYSIZE(ProceduralMeshComponent::shapeNames); ix++) {
							const bool is_selected = (chosen_index == ix);
							if (ImGui::Selectable(ProceduralMeshComponent::shapeNames[ix], is_selected)) {
								pMesh.parameters.shape = (ProceduralMeshComponent::Shape)ix;
							}
							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected) ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					switch (pMesh.parameters.shape) {
					case ProceduralMeshComponent::Shape::Box:
						if (DrawVec3Control("Dimensions", pMesh.parameters.box.dimensions, 1.0f)) {
							pMesh.GenerateMesh();
						}
						break;
					case ProceduralMeshComponent::Shape::Torus:
						if (DrawVec3Control("Dimensions", pMesh.parameters.box.dimensions)) {
							pMesh.GenerateMesh();
						}
						break;
					}
				}
			}
			else if (info != entt::type_id<TagComponent>()) { // default
				ImGui::Text("Component '%s' has no UI yet", info.name().data());
			}
		});

		if (ImGui::Button("Add Component")) { ImGui::OpenPopup("AddComponent"); }
		if (ImGui::BeginPopup("AddComponent")) {
			if (!selectedObject.any_of<MeshComponent>() && ImGui::MenuItem("Mesh Component")) {
				selectedObject.emplace<MeshComponent>();
			}
			if (!selectedObject.any_of<ProceduralMeshComponent>() && ImGui::MenuItem("Procedural Mesh Component")) {
				selectedObject.emplace<ProceduralMeshComponent>();
			}
			if (!selectedObject.any_of<MeshRendererComponent>() &&  ImGui::MenuItem("Mesh Renderer Component")) {
				selectedObject.emplace<MeshRendererComponent>();
			}
			ImGui::EndPopup();
		}
	}

	ImGui::End();
}