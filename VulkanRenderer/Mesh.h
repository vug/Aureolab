#pragma once

#include "Types.h"

#include <glm/glm.hpp>

#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec4 color;
};

struct Mesh {
	std::vector<Vertex> vertices;
	AllocatedBuffer vertexBuffer;
};