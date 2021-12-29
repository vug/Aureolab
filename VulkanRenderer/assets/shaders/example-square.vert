#version 450

layout(location = 0) out vec3 fragColor;

vec2 positions[6] = vec2[](
    vec2(-0.5, -0.5),
    vec2(+0.5, -0.5),
    vec2(+0.5, +0.5),

    vec2(-0.5, -0.5),
    vec2(+0.5, +0.5),
    vec2(-0.5, +0.5)
);

vec3 colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),

    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 1.0)
);

void main() {
    // 0.75 is for moving the plane a little bit into the screen
    // in case it is going to be rendered together with other mesh
    gl_Position = vec4(positions[gl_VertexIndex], 0.75, 1.0);
    fragColor = colors[gl_VertexIndex];
}