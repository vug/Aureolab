#include "Scene.h"

#include "Components.h"

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

void Scene::Save() {
	// empty storage
	storage.str(std::string());
	storage.clear();

	cereal::JSONOutputArchive output{ storage };
	entt::snapshot{ registry }.entities(output).component<TagComponent, TransformComponent, MeshComponent>(output);
}

void Scene::Load() {
	cereal::JSONInputArchive input{ storage };
	registry.clear();
	entt::snapshot_loader{ registry }.entities(input).component<TagComponent, TransformComponent, MeshComponent>(input).orphans();

	// rewind storage
	storage.clear();
	storage.seekg(0, std::ios_base::beg);
}


