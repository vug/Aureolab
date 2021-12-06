#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D tex1;

void main() {
    vec3 color = texture(tex1, vTexCoord).xyz;
    outColor = vec4(color, 1.0);
}