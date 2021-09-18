#type vertex
#version 460 core

uniform mat4 MVP;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_Color;

varying vec4 pos;
varying vec3 normal;
varying vec2 uv;
varying vec4 color;

void main() {
    gl_Position = MVP * vec4(a_Position, 1.0);
    pos = gl_Position;
    normal = a_Normal;
    uv = a_TexCoord;
    color = a_Color;
}


#type fragment
#version 460 core

varying vec4 pos;
varying vec3 normal;
varying vec2 uv;
varying vec4 color;

void main() {
    //gl_FragColor = vec4(vec3(1.0) * pos.w / 100, 1.0);
    gl_FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    //gl_FragColor = vec4(uv.x, uv.y, 0.0, 1.0);
}