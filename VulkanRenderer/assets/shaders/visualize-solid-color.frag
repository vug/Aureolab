#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 color = vec3(1.0);
    outColor = vec4(color, 1.0);
}