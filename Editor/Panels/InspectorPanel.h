#pragma once

#include "Scene/Scene.h"

class InspectorPanel {
public:
	InspectorPanel() = default;
	InspectorPanel(Scene* scene, EntityHandle* selectedObject)
		: scene(scene), selectedObject(selectedObject) {}

	void OnImGuiRender();

private:
	// references to EditorLayer's members
	Scene* scene;
	EntityHandle* selectedObject;
};