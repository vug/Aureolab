#pragma once

#include <glm/glm.hpp>

#include <numbers>

namespace Math {
	static float PI = std::numbers::pi;
	static float TAU = 2.0f * std::numbers::pi;

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	glm::mat4 ComposeTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);
}
