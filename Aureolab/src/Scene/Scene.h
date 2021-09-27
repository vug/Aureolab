#pragma once

#include <entt/entt.hpp>

using EntityHandle = entt::basic_handle<entt::entity>;

class Scene {
public:
	EntityHandle CreateEntity();

	template<typename... Comps>
	auto View() {
		auto view = registry.view<Comps...>();
		return view;
	}

private:
	entt::registry registry;
};