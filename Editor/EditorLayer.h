#pragma once

#include "Panels/MainMenuBar.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/Shader.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/FrameBuffer.h"
#include "Renderer/EditorCamera.h"
#include "Scene/Scene.h"

#include <vector>

class EditorLayer : public Layer {
public:
	EditorLayer() : Layer("Editor Layer") { }

	virtual void OnAttach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnDetach() override;

	virtual void OnEvent(Event& ev) override;
	virtual void OnImGuiRender() override;

private:
	Shader* shader = nullptr;
	Shader* selectionShader = nullptr;
	Shader* solidColorShader = nullptr;
	UniformBuffer* viewUbo = nullptr;
	UniformBuffer* lightsUbo = nullptr;
	FrameBuffer* viewportFbo = nullptr;
	FrameBuffer* selectionFbo = nullptr;
	int mouseX, mouseY;
	EditorCamera* camera = nullptr;

	Scene scene;
	EntityHandle selectedObject = {};
	EntityHandle hoveredObject = {};
	int hoveredEntityId = -3;

	MainMenuBar mainMenuBar{ scene };
	SceneHierarchyPanel hierarchyPanel{ scene, selectedObject };
	InspectorPanel inspectorPanel{ scene, selectedObject, hoveredObject };
	ViewportPanel viewportPanel{ viewportFbo, selectionFbo, hoveredEntityId, camera, selectedObject, hoveredObject, mouseX, mouseY };

	std::vector<float> frameRates = std::vector<float>(120);
};