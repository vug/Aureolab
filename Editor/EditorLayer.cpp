#include "EditorLayer.h"

void EditorLayer::OnAttach() {
	// in case we'll see an area not behind any ImWindow
	GraphicsAPI::Get()->Enable(GraphicsAbility::DepthTest);
	auto turquoise = glm::vec4{ 64, 224, 238, 1 } / 255.0f;
	GraphicsAPI::Get()->SetClearColor(turquoise);

	shader = Shader::Create("assets/shaders/BasicShader.glsl");
	fbo = FrameBuffer::Create(100, 100); // arguments does not matter since FBO's going to be resized

	camera = new EditorCamera(45, aspect, 0.01f, 100);

	// Hard-coded example scene
	using ObjectData = struct {
		std::string name;
		glm::vec3 pos;
		std::string meshFilePath;
		MeshRendererComponent::Visualization viz;
	};
	std::vector<ObjectData> sceneData = {
		{ "monkey1", { 0.75, 0.5, 0.0 }, "assets/models/suzanne_smooth.obj", MeshRendererComponent::Visualization::Normal, },
		{ "monkey2", { -0.5, -0.1, 0.0 }, "assets/models/suzanne.obj", MeshRendererComponent::Visualization::UV, },
		{ "torus", { 0.1, -0.4, 0.7 }, "assets/models/torus_smooth.obj", MeshRendererComponent::Visualization::Depth, },
	};
	for (ObjectData& obj : sceneData) {
		auto ent = scene.CreateEntity(obj.name);
		auto& transform = ent.get<TransformComponent>();
		transform.translation = obj.pos;
		transform.rotation = obj.pos;
		transform.scale = { 0.4, 0.4, 0.4 };
		ent.emplace<MeshComponent>(obj.meshFilePath);
		ent.emplace<MeshRendererComponent>(obj.viz);
	}
	//scene.LoadFromFile("assets/scenes/first.scene");
}


void EditorLayer::OnUpdate(float ts) {
	std::shift_left(frameRates.begin(), frameRates.end(), 1);
	frameRates[frameRates.size() - 1] = 1.0f / ts;

	camera->OnUpdate(ts);
	projection = camera->GetProjection();
	view = camera->GetViewMatrix();

	GraphicsAPI::Get()->Clear();

	fbo->Bind(); // Render into viewportFBO
	GraphicsAPI::Get()->SetClearColor({ 0, 0, 0, 1 });
	GraphicsAPI::Get()->Clear();
	shader->Bind();
	auto query = scene.View<TransformComponent, MeshComponent, MeshRendererComponent>();
	for (const auto& [ent, transform, mesh, meshRenderer] : query.each()) {
		glm::vec3& translation = transform.translation;
		glm::mat4 model = Math::ComposeTransform(transform.translation, transform.rotation, transform.scale);
		glm::mat4 modelView = view * model;
		glm::mat4 modelViewProjection = projection * modelView;
		glm::mat4 normalMatrix = glm::inverse(modelView);
		shader->UploadUniformMat4("u_ModelViewPerspective", modelViewProjection);
		shader->UploadUniformMat4("u_NormalMatrix", normalMatrix);
		shader->UploadUniformMat4("u_View", view);

		shader->UploadUniformInt("u_RenderType", (int)meshRenderer.visualization);
		shader->UploadUniformFloat4("u_SolidColor", meshRenderer.solidColor);
		shader->UploadUniformFloat("u_DepthMax", meshRenderer.depthParams.max);
		shader->UploadUniformFloat("u_DepthPow", meshRenderer.depthParams.pow);

		VertexArray* vao = mesh.vao;
		if (vao == nullptr) { continue; }
		GraphicsAPI::Get()->DrawArrayTriangles(*vao);
	}
	fbo->Unbind();
}

void EditorLayer::OnDetach() {
	delete fbo;
}

void EditorLayer::OnEvent(Event& ev) {
	auto dispatcher = EventDispatcher(ev);
	dispatcher.Dispatch<MouseScrolledEvent>(AL_BIND_EVENT_FN(EditorLayer::OnMouseScrolled));
	dispatcher.Dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
}

void EditorLayer::OnMouseScrolled(MouseScrolledEvent& ev) {
	if (isViewportPanelHovered) camera->OnMouseScroll(ev.GetXOffset(), ev.GetYOffset());
}

void EditorLayer::OnKeyPressed(KeyPressedEvent& ev) {
	switch (ev.GetKeyCode()) {
		// Transform Gizmos
	case 90: // Z
		gizmoShouldShow = true;
		gizmoType = ImGuizmo::OPERATION::TRANSLATE;
		break;
	case 88: // X
		gizmoShouldShow = true;
		gizmoType = ImGuizmo::OPERATION::ROTATE;
		break;
	case 67: // C
		gizmoShouldShow = true;
		gizmoType = ImGuizmo::OPERATION::SCALE;
		break;
	case 86: // V
		gizmoShouldShow = false;
		break;
	}
}

void EditorLayer::OnImGuiRender() {
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_None);

	// Left Column
	hierarchyPanel.OnImGuiRender();

	// MIDDLE COLUMN: Viewport Panel displays content of viewportFBO
	static ImVec2 viewportPanelAvailRegionPrev;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground); // ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
	ImVec2 viewportPanelAvailRegion = ImGui::GetContentRegionAvail();
	bool isViewportPanelResized = viewportPanelAvailRegion.x != viewportPanelAvailRegionPrev.x || viewportPanelAvailRegion.y != viewportPanelAvailRegionPrev.y;
	if (isViewportPanelResized) {
		fbo->Resize((int)viewportPanelAvailRegion.x, (int)viewportPanelAvailRegion.y);
		glViewport(0, 0, (int)viewportPanelAvailRegion.x, (int)viewportPanelAvailRegion.y);
		camera->SetViewportSize(viewportPanelAvailRegion.x, viewportPanelAvailRegion.y);
	}
	ImGui::Image((void*)(intptr_t)fbo->GetColorAttachmentRendererID(0), ImVec2(viewportPanelAvailRegion.x, viewportPanelAvailRegion.y), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
	aspect = viewportPanelAvailRegion.x / viewportPanelAvailRegion.y;
	isViewportPanelHovered = ImGui::IsWindowHovered();

	if (selectedObject && gizmoShouldShow) {
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewportPanelAvailRegion.x, viewportPanelAvailRegion.y);

		auto& tc = selectedObject.get<TransformComponent>();
		auto transformMatrix = Math::ComposeTransform(tc.translation, tc.rotation, tc.scale);

		// Snapping
		const bool shouldSnap = Input::Get()->IsKeyPressed(340);
		// 45 degrees for rotation, 0.5m for translate and scale
		float snapValue = gizmoType == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f;
		float snapValues[3] = { snapValue, snapValue, snapValue };

		ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
			gizmoType, ImGuizmo::LOCAL, glm::value_ptr(transformMatrix), nullptr, shouldSnap ? snapValues : nullptr);

		if (ImGuizmo::IsUsing()) {
			glm::vec3 translation, rotation, scale;
			Math::DecomposeTransform(transformMatrix, translation, rotation, scale);
			tc.translation = translation;
			// To prevent gimble-lock
			glm::vec3 deltaRotation = rotation - tc.rotation;
			tc.rotation += deltaRotation;
			tc.scale = scale;
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();

	// RIGHT COLUMN: Inspector Panel
	static bool shouldShowDemo = false;
	ImGui::Begin("Inspector");
	ImGui::Text("Components");
	if (selectedObject) {
		ImGui::InputText("Tag", &selectedObject.get<TagComponent>().tag);
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
	}
	ImGui::End();

	ImGui::Begin("Stats");
	ImGui::Text("Stats:\n"
		"mainViewportSize: (%.1f, %.1f)\n"
		"viewportPanelAvailRegion: (%.1f, %.1f)\n"
		"",
		viewport->Size.x, viewport->Size.y,
		viewportPanelAvailRegion.x, viewportPanelAvailRegion.y
	);

	if (ImGui::CollapsingHeader("FPS", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PlotLines("", frameRates.data(), (int)frameRates.size());
		const auto [minIt, maxIt] = std::minmax_element(frameRates.begin(), frameRates.end());
		ImGui::Text("[%.1f %.1f]", *minIt, *maxIt);
		static bool isVSync = false;
		if (ImGui::Checkbox("VSync", &isVSync)) { GraphicsContext::Get()->SetVSync(isVSync); }
	}

	if (ImGui::CollapsingHeader("Editor Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Yaw, Pitch, Roll: (%.2f, %.2f, %.2f)", camera->GetYaw(), camera->GetPitch(), camera->GetRoll());
		ImGui::Text("Pos: (%.1f, %.1f, %.1f)", camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
		ImGui::Text("Target: (%.1f, %.1f, %.1f)", camera->GetFocalPoint().x, camera->GetFocalPoint().y, camera->GetFocalPoint().z);
		ImGui::Text("Distance: %.1f", camera->GetDistance());
		float fov = camera->GetFOV();
		if (ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f)) { camera->SetFOV(fov); }
		ImGui::SliderFloat("Roll", camera->GetRefRoll(), 0.0f, 3.141593f);
	}

	ImGui::Separator();
	ImGui::Checkbox("Show Demo Window", &shouldShowDemo);
	viewportPanelAvailRegionPrev = viewportPanelAvailRegion;
	ImGui::End();

	if (shouldShowDemo) ImGui::ShowDemoWindow();
}

