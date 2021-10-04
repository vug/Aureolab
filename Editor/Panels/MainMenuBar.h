#pragma once

#include "Scene/Scene.h"

class MainMenuBar {
public:
	MainMenuBar() = default;
	MainMenuBar(Scene & scene)
		: scene(scene) {}

	void OnImGuiRender();
private:
	// references to EditorLayer's members
	Scene& scene;
};