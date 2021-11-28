#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec4 vColor;

layout (push_constant) uniform Constants {
	vec4 data;
	mat4 modelViewProjection;
} pushConstants;

layout (location = 0) out vec4 outColor;

void main() {
	gl_Position = pushConstants.modelViewProjection * vec4(vPosition, 1.0f);
	outColor = vColor;
}