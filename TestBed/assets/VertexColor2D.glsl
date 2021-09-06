#type vertex
#version 460 core

uniform mat4 MVP;

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vCol;

varying vec3 color;

void main() {
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}


#type fragment
#version 460 core

varying vec3 color;

void main() {
    gl_FragColor = vec4(color, 1.0);
}