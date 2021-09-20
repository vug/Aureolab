#type vertex
#version 460 core

uniform mat4 u_MVP;
uniform mat4 u_NormalMatrix;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_Color;

out vec4 position;
out vec3 normal;
out vec2 uv;
out vec4 color;

out vec3 worldPosition;
out vec3 worldNormal;

void main() {
    position = u_MVP * vec4(a_Position, 1.0);
    normal = (u_NormalMatrix * vec4(a_Normal, 1.0)).xyz; // not normalized yet
    uv = a_TexCoord;
    color = a_Color;

    worldPosition = a_Position;
    worldNormal = a_Normal;

    gl_Position = position;
}


#type fragment
#version 460 core

uniform int u_RenderType = 1; // { SolidColor, Normal, UV, Depth }
uniform vec4 u_SolidColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform float u_MaxDepth = 100.0;
uniform vec3 u_LightPosition = vec3(0.0, 10.0, 0.0);
uniform vec4 u_DiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);

in vec4 position;
in vec3 normal;
in vec2 uv;
in vec4 color;
in vec3 worldPosition;
in vec3 worldNormal;

void main() {
    switch (u_RenderType) {
    case 0:
        gl_FragColor = u_SolidColor;
        break;
    case 1:
        gl_FragColor = vec4(worldNormal * 0.5 + 0.5, 1.0);
        break;
    case 2:
        gl_FragColor = vec4(uv.x, uv.y, 0.0, 1.0);
        break;
    case 3:
        gl_FragColor = vec4(vec3(1.0) * position.z / u_MaxDepth, 1.0);
        break;
    case 4:
        vec3 nNormal = normalize(normal);
        vec3 rayDir = normalize(u_LightPosition - worldPosition);
        float value = max(0.0, dot(rayDir, nNormal));
        gl_FragColor = vec4(u_DiffuseColor.rgb * value, u_DiffuseColor.a);
        break;
    }
}