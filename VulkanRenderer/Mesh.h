#pragma once

#include "Types.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>

struct VertexInputDescription {
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec4 color;

	static VertexInputDescription GetVertexDescription();
};

struct Mesh {
	std::vector<Vertex> vertices;
	AllocatedBuffer vertexBuffer;
};