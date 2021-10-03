#pragma once

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"

#include "Core/Layer.h"
#include "Events/Event.h"
#include "Renderer/Shader.h"
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
	FrameBuffer* fbo = nullptr;
	EditorCamera* camera = nullptr;

	Scene scene;
	EntityHandle selectedObject = {};

	SceneHierarchyPanel hierarchyPanel{ scene, selectedObject };
	InspectorPanel inspectorPanel{ scene, selectedObject };
	ViewportPanel viewportPanel{ fbo, camera, selectedObject };

	std::vector<float> frameRates = std::vector<float>(120);
};