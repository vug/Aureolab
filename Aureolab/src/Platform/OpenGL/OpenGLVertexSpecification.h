#pragma once

#include "Renderer/VertexSpecification.h"

#include <glad/glad.h>

// Helper to convert AureoLab data type to OpenGL data type
GLenum ALTypeToGLType(VertexAttributeType alType);

// Helper to get OpenGL type size of AureoLab data type
unsigned int TypeSize(VertexAttributeType type);
