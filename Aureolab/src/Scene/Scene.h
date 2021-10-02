#pragma once

#include "Core/Log.h"
#include "Components.h"

#include <entt/entt.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

#include <string>


using EntityHandle = entt::basic_handle<entt::entity>;

class Scene {
public:
	EntityHandle CreateEntity(const std::string& name);
	void DestroyEntity(EntityHandle ent);

	template<typename... Comps>
	auto View() {
		auto view = registry.view<Comps...>();
		return view;
	}

	void Visit(EntityHandle ent, std::function<void(const entt::type_info)> func);

	EntityHandle GetHandle(entt::entity ent);

	void Save() {
		// empty storage
		storage.str(std::string());
		storage.clear();
		cereal::JSONOutputArchive output{ storage };
		entt::snapshot{ registry }.entities(output).component<TagComponent, TransformComponent, MeshComponent>(output);
	}

	void Load() {
		cereal::JSONInputArchive input{ storage };
		registry.clear();
		entt::snapshot_loader{ registry }.entities(input).component<TagComponent, TransformComponent, MeshComponent>(input).orphans();
		// rewind storage
		storage.clear();
		storage.seekg(0, std::ios_base::beg);
	}

private:
	entt::registry registry;
	std::stringstream storage;
};