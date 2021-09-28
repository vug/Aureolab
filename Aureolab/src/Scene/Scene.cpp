#include "Scene.h"

#include "Components.h"

EntityHandle Scene::CreateEntity(const std::string& name) {
	entt::entity ent = registry.create();
	EntityHandle handle = { registry, ent };
	handle.emplace<TransformComponent>();
	handle.emplace<TagComponent>(name);
	return handle;
}

void Scene::Visit(EntityHandle ent, std::function<void(const entt::type_info)> func) {
	registry.visit(ent, func);
}

EntityHandle Scene::GetHandle(entt::entity ent) {
	return EntityHandle{ registry, ent };
}
