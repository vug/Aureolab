#pragma once

enum class VertexAttributeSemantic {
	Position, 
	Color,
	Normal,
	UV,
};

enum class VertexAttributeType {
	int8,
	uint8,
	int16,
	uint16,
	int32,
	uint32,
	float32,
	float64,
};

class VertexSpecification {
public:
	unsigned int index;
	VertexAttributeSemantic semantic;
	VertexAttributeType type;
	int numComponents;
	bool normalized = false;
};

class VertexBuffer {
public:
	static VertexBuffer* Create();
};