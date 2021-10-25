#type vertex
#version 460 core

uniform mat4 u_ModelViewPerspective;

layout(location = 0) in vec3 a_Position;

void main() {
    gl_Position = u_ModelViewPerspective * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

uniform vec4 u_Color = vec4(1.0, 1.0, 0.0, 1.0);

layout (location = 0) out vec4 outColor;

void main() {
    outColor = u_Color;
}