#pragma once

#include "Renderer/VertexBuffer.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct BasicVertex {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
    glm::vec2 texCoord = { 0.0f, 0.0f };
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

// To be in sync with BasicShader
static std::vector<VertexAttributeSpecification> BasicVertexAttributeSpecs = {
    VertexAttributeSpecification{ 0, VertexAttributeSemantic::Position, VertexAttributeType::float32, 3, false},
    VertexAttributeSpecification{ 1, VertexAttributeSemantic::Normal, VertexAttributeType::float32, 3, false},
    VertexAttributeSpecification{ 2, VertexAttributeSemantic::UV, VertexAttributeType::float32, 2, false},
    VertexAttributeSpecification{ 3, VertexAttributeSemantic::Color, VertexAttributeType::float32, 4, false},
};

std::vector<BasicVertex> LoadOBJ(const std::string& filepath);

std::vector<BasicVertex> GenerateBox(glm::vec3 dimensions);
std::vector<BasicVertex> GenerateTorus(float outerRadius, int outerSegments, float innerRadius, int innerSegments);