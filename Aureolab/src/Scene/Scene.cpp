#include "Scene.h"

#include "Components.h"

EntityHandle Scene::CreateEntity() {
	entt::entity ent = registry.create();
	EntityHandle handle = { registry, ent };
	handle.emplace<TransformComponent>();
	handle.emplace<TagComponent>();
	return handle;
}
