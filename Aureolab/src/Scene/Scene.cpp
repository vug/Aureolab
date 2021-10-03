#include "Scene.h"

#include "Components.h"

#include <fstream>

EntityHandle Scene::CreateEntity(const std::string& name) {
	entt::entity ent = registry.create();
	EntityHandle handle = { registry, ent };
	handle.emplace<TransformComponent>();
	handle.emplace<TagComponent>(name);
	return handle;
}

void Scene::DestroyEntity(EntityHandle ent) {
	registry.destroy(ent);
}

void Scene::Visit(EntityHandle ent, std::function<void(const entt::type_info)> func) {
	registry.visit(ent, func);
}

EntityHandle Scene::GetHandle(entt::entity ent) {
	return EntityHandle{ registry, ent };
}

void Scene::SaveToFile(const std::string& filepath) {
	std::ofstream file(filepath);
	cereal::JSONOutputArchive output{ file };
	entt::snapshot{ registry }.entities(output).component<TagComponent, TransformComponent, MeshComponent, MeshRendererComponent>(output);
}

void Scene::LoadFromFile(const std::string& filepath) {
	registry.clear();
	std::ifstream file(filepath);
	cereal::JSONInputArchive input{ file };
	entt::snapshot_loader{ registry }.entities(input).component<TagComponent, TransformComponent, MeshComponent, MeshRendererComponent>(input).orphans();
}

void Scene::SaveToMemory() {
	// empty storage
	storage.str(std::string());
	storage.clear();

	cereal::JSONOutputArchive outputMemory{ storage };
	entt::snapshot{ registry }.entities(outputMemory).component<TagComponent, TransformComponent, MeshComponent, MeshRendererComponent>(outputMemory);
}

void Scene::LoadFromMemory() {
	cereal::JSONInputArchive input{ storage };
	registry.clear();
	entt::snapshot_loader{ registry }.entities(input).component<TagComponent, TransformComponent, MeshComponent, MeshRendererComponent>(input).orphans();
	// rewind storage
	storage.clear();
	storage.seekg(0, std::ios_base::beg);
}


