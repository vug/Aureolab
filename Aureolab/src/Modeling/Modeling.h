#pragma once

#include "Renderer/VertexBuffer.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct BasicVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

std::vector<BasicVertex> LoadOBJ(const std::string& filepath);