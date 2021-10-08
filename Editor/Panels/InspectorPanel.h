#pragma once

#include "Scene/Scene.h"

class InspectorPanel {
public:
	InspectorPanel() = default;
	InspectorPanel(Scene& scene, EntityHandle& selectedObject, EntityHandle& hoveredObject)
		: scene(scene), selectedObject(selectedObject), hoveredObject(hoveredObject) {}

	void OnImGuiRender();

private:
	// references to EditorLayer's members
	Scene& scene;
	EntityHandle& selectedObject;
	EntityHandle& hoveredObject;
};