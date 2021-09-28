#include "Scene.h"

#include "Components.h"

EntityHandle Scene::CreateEntity(const std::string& name) {
	entt::entity ent = registry.create();
	EntityHandle handle = { registry, ent };
	handle.emplace<TransformComponent>();
	handle.emplace<TagComponent>(name);
	return handle;
}
