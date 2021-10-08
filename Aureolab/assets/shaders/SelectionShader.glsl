#type vertex
#version 460 core

uniform mat4 u_ModelViewPerspective;

layout(location = 0) in vec3 a_Position;

void main() {
    gl_Position = u_ModelViewPerspective * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core

uniform int u_EntityID = -2; // value when uniform is not provided

layout (location = 0) out int entityID;

void main() {
    entityID = u_EntityID;
}