#type vertex
#version 460 core

layout(binding = 0, std140) uniform ViewMatrices {
	mat4 u_View;
	mat4 u_Projection;
};

uniform mat4 u_ModelViewPerspective;
uniform mat4 u_ModelView;
uniform mat4 u_Model;
uniform mat4 u_NormalMatrix;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_Color;

out vec4 position;
out vec3 preNormal;
out vec2 uv;
out vec4 color;

out vec3 positionView;
out vec3 modelNormal;

void main() {
    position = u_ModelViewPerspective * vec4(a_Position, 1.0);
    preNormal = (u_NormalMatrix * vec4(a_Normal, 1.0)).xyz; // not normalized yet
    uv = a_TexCoord;
    color = a_Color;

    positionView = (u_ModelView * vec4(a_Position, 1.0)).xyz;
    modelNormal = a_Normal;

    gl_Position = position;
}


#type fragment
#version 460 core

layout(binding = 0, std140) uniform ViewMatrices {
	mat4 u_View;
	mat4 u_Projection;
};

uniform int u_RenderType = 1; // { SolidColor, Normal, UV, Depth }
uniform vec4 u_SolidColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform float u_DepthMax = 5.0;
uniform float u_DepthPow = 2.0;
uniform vec3 u_LightPosition = vec3(0.0, 10.0, 0.0);
uniform vec3 u_LightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 u_LightAttenuation = vec3(0.0, 1.0, 0.0);
uniform vec4 u_DiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 u_SkyColor = vec3(0.0, 0.0, 1.0);
uniform vec3 u_GroundColor = vec3(0.0, 1.0, 0.0);

in vec4 position;
in vec3 preNormal;
in vec2 uv;
in vec4 color;
in vec3 positionView;
in vec3 modelNormal;

layout (location = 0) out vec4 outColor;

void main() {
    vec3 normal = normalize(preNormal);
    switch (u_RenderType) {
    case 0: // Solid Color
        outColor = u_SolidColor;
        break;
    case 1: // Normal
        outColor = vec4(modelNormal * 0.5 + 0.5, 1.0);
        break;
    case 2: // UV
        outColor = vec4(uv.x, uv.y, 0.0, 1.0);
        break;
    case 3: // Depth
        outColor = vec4(vec3(1.0) - pow(position.z / u_DepthMax, u_DepthPow), 1.0);
        break;
    case 4: // Vertex Color
        outColor = color;
        break;
    case 5: // Front & Back Faces
        if (gl_FrontFacing) { outColor = vec4(1.0, 0.0, 0.0, 1.0); }
        else { outColor = vec4(0.0, 0.0, 1.0, 1.0); }
        break;
    case 6: // Checkers
        vec2 p = uv * 10.0;
        outColor = vec4(vec3(int(p.x) % 2 ^ int(p.y) % 2), 1.0); // bitwise XOR for checkers pattern
        break;
    case 7: // Point Light
        vec3 lightPositionView = (u_View * vec4(u_LightPosition, 1.0)).xyz;
        vec3 relativeLightPosition = lightPositionView - positionView;
        float lightDistance = length(relativeLightPosition);
        vec3 lightDirection = relativeLightPosition / lightDistance; // normalize
        float attenuation = 1.0 / (u_LightAttenuation.x + u_LightAttenuation.y * lightDistance + u_LightAttenuation.z * lightDistance * lightDistance);
        float diffuseValue = max(0.0, dot(lightDirection, normal));
        vec3 lightScattered = u_LightColor * diffuseValue * attenuation;
        vec3 rgb = u_DiffuseColor.rgb * lightScattered;
        outColor = vec4(rgb, u_DiffuseColor.a);
        break;
    case 8: // Hemispherical Light
        float costheta = dot(normal, vec3(0.0, 1.0, 0.0));
        float a = costheta * 0.5 + 0.5;
        outColor = vec4(mix(u_GroundColor, u_SkyColor, a), 1.0);
        break;
    }
}