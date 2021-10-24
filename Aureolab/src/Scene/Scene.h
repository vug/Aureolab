#pragma once

#include "Core/Log.h"
#include "Components.h"

#include <entt/entt.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <glm/glm.hpp>

#include <string>


using EntityHandle = entt::basic_handle<entt::entity>;

class Scene {
public:
	EntityHandle CreateEntity(const std::string& name);
	void DestroyEntity(EntityHandle ent);
	void DuplicateEntity(const EntityHandle& entity);

	template<typename... Comps>
	auto View() {
		auto view = registry.view<Comps...>();
		return view;
	}

	void Visit(EntityHandle ent, std::function<void(const entt::type_info)> func);

	EntityHandle GetHandle(entt::entity ent);

	void New();
	void SaveToFile(const std::string& filepath);
	void LoadFromFile(const std::string& filepath);
	void SaveToMemory();
	void LoadFromMemory();

	glm::vec4 ambientColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	glm::vec4 backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f };

private:
	entt::registry registry;
	std::stringstream storage;
};