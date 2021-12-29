#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec4 vColor;

layout(set = 0, binding = 0) uniform CameraBuffer {
	mat4 view;
	mat4 proj;
} cameraData;

layout (push_constant) uniform Constants {
	vec4 data;
	mat4 transform; // model
} pushConstants;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;
layout (location = 2) out vec3 outNormal;

void main() {
	gl_Position = cameraData.proj * cameraData.view * pushConstants.transform * vec4(vPosition, 1.0f);
	outColor = vColor;
	outTexCoord = vTexCoord;
	outNormal = vNormal;
}