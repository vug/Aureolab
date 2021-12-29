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
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
	glm::vec2 texCoord = { 0.0f, 0.0f };
	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	static VertexInputDescription GetVertexDescription();
};

struct Mesh {
	std::vector<Vertex> vertices;
	AllocatedBuffer vertexBuffer;

	bool LoadFromOBJ(const char* filename);
	void MakeTriangle();
	void MakeQuad();
};

struct MeshPushConstants {
	struct PushConstant1 {
		glm::vec4 data;
		glm::mat4 modelViewProjection;
	};

	struct PushConstant2 {
		glm::vec4 data; // unspecified vec4 for arbitrary data
		glm::mat4 transform;
	};

	template <typename TStruct>
	static VkPushConstantRange GetPushConstantRange();
};
// Need one of these per PushConstant struct
template VkPushConstantRange MeshPushConstants::GetPushConstantRange<MeshPushConstants::PushConstant1>();
template VkPushConstantRange MeshPushConstants::GetPushConstantRange<MeshPushConstants::PushConstant2>();