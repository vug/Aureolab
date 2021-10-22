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
			{ "monkey2", { -0.5, -0.1, 0.0 }, "assets/models/suzanne.obj", MeshRendererComponent::Visualization::HemisphericalLight, },
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

		auto procedural1 = scene.CreateEntity("procedural1");
		TransformComponent& transform = procedural1.get<TransformComponent>();
		transform.translation.y = 0.75;
		transform.rotation = { +0.93, -0.82, -0.77 };
		auto& pmComp1 = procedural1.emplace<ProceduralMeshComponent>();
		pmComp1.parameters.box.dimensions = { 0.4f, 0.6f, 0.2f };
		pmComp1.GenerateMesh();
		procedural1.emplace<MeshRendererComponent>(MeshRendererComponent::Visualization::VertexColor);

		auto procedural2 = scene.CreateEntity("procedural2");
		TransformComponent& tTransform = procedural2.get<TransformComponent>();
		tTransform.translation = { 0.24, 0.03, -0.43 };
		tTransform.rotation = { -0.25, -0.33, 0.30 };
		auto& pmComp2 = procedural2.emplace<ProceduralMeshComponent>();
		pmComp2.parameters.shape = ProceduralMeshComponent::Shape::Torus;
		pmComp2.parameters.torus = { 0.333, 12, 0.166, 8 };
		pmComp2.GenerateMesh();
		procedural2.emplace<MeshRendererComponent>(MeshRendererComponent::Visualization::Checkers);
	}
};