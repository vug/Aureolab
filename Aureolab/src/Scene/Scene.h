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

private:
	entt::registry registry;
};