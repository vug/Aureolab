#type vertex
#version 460 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_UV;

out vec2 uv;

void main() {
    gl_Position = vec4(a_Position, 0.0, 1.0);
    uv = a_UV;
}


#type fragment
#version 460 core

in vec2 uv;

void main() {
    gl_FragColor = vec4(uv.x, uv.y, 0.0, 1.0);
}