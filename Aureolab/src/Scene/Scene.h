#pragma once

#include <entt/entt.hpp>

#include <string>

using EntityHandle = entt::basic_handle<entt::entity>;

class Scene {
public:
	EntityHandle CreateEntity(const std::string& name);

	template<typename... Comps>
	auto View() {
		auto view = registry.view<Comps...>();
		return view;
	}

	void Visit(EntityHandle ent, std::function<void(const entt::type_info)> func);

	EntityHandle GetHandle(entt::entity ent);

private:
	entt::registry registry;
};