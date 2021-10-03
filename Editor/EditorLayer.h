#pragma once

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"

#include "Core/Layer.h"
#include "Core/Input.h"
#include "Core/GraphicsContext.h"
#include "Events/Event.h"
#include "Renderer/GraphicsAPI.h"
#include "Renderer/Shader.h"
#include "Renderer/FrameBuffer.h"
#include "Renderer/EditorCamera.h"
#include "Modeling/Modeling.h"
#include "Scene/Scene.h"
#include "Scene/Components.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <cereal/cereal.hpp>

#include <algorithm>
#include <string>
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

	std::vector<float> frameRates = std::vector<float>(120);

	Scene scene;
	EntityHandle selectedObject = {};

	SceneHierarchyPanel hierarchyPanel{ &scene, &selectedObject };
	InspectorPanel inspectorPanel{ &scene, &selectedObject };
	ViewportPanel viewportPanel{ fbo, camera, selectedObject };
};