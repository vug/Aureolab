#include "Scene/Components.h"
#include "Scene/Scene.h"

#include <glm/glm.hpp>

#include <string>

class ExampleScene {
public:
	// Hard-coded, programmatically generated example scene. This'll help to serialize a scene with new features.
	static void PopulateScene(Scene& scene) {
		using ObjectData = struct {
			std::string name;
			glm::vec3 pos;
			std::string meshFilePath;
			MeshRendererComponent::Visualization viz;
		};
		std::vector<ObjectData> sceneData = {
			{ "monkey1", { 0.75, 0.5, 0.0 }, "assets/models/suzanne_smooth.obj", MeshRendererComponent::Visualization::Normal, },
			{ "monkey2", { -0.5, -0.1, 0.0 }, "assets/models/suzanne.obj", MeshRendererComponent::Visualization::Depth, },
			{ "torus", { 0.1, -0.4, 0.7 }, "assets/models/torus_smooth.obj", MeshRendererComponent::Visualization::UV, },
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

		auto procedural = scene.CreateEntity("procedural1");
		TransformComponent& transform = procedural.get<TransformComponent>();
		transform.translation.y = 0.75;
		transform.rotation = { +0.93, -0.82, -0.77 };
		auto& pmComp = procedural.emplace<ProceduralMeshComponent>();
		pmComp.parameters.box.dimensions = { 0.4f, 0.6f, 0.2f };
		pmComp.GenerateMesh();
		procedural.emplace<MeshRendererComponent>(MeshRendererComponent::Visualization::VertexColor);
	}
};