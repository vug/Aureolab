#pragma once

#include "Scene/Scene.h"

#include <imgui.h>

class InspectorPanel {
public:
	InspectorPanel() = default;
	InspectorPanel(Scene& scene, EntityHandle& selectedObject, EntityHandle& hoveredObject)
		: scene(scene), selectedObject(selectedObject), hoveredObject(hoveredObject) {}

	void OnImGuiRender();

private:
	template <class Component>
	void AddComponentMenuItem(const char* name) {
		if (!selectedObject.any_of<Component>() && ImGui::MenuItem(name)) {
			selectedObject.emplace<Component>();
		}
	}
	// references to EditorLayer's members
	Scene& scene;
	EntityHandle& selectedObject;
	EntityHandle& hoveredObject;
};