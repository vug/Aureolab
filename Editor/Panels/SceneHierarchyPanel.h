#pragma once

#include "Scene/Scene.h"

class SceneHierarchyPanel {
public:
	SceneHierarchyPanel() = default;
	SceneHierarchyPanel(Scene* scene, EntityHandle* selectedObject) 
		: scene(scene), selectedObject(selectedObject) {}

	void OnImGuiRender();

private:
	// references to EditorLayer's members
	Scene* scene;
	EntityHandle* selectedObject;
};