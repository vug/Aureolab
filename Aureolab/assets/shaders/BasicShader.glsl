#type vertex
#version 460 core

layout(std140) uniform ViewMatrices {
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

out vec3 positionView;
out vec3 positionWorld;
out vec4 positionFrag;
out vec3 normalModel;
out vec3 preNormalWorld;
out vec2 uv;
out vec4 color;

void main() {
    positionWorld = vec3(u_Model * vec4(a_Position, 1.0));
    positionView = vec3(u_ModelView * vec4(a_Position, 1.0));
    positionFrag = u_ModelViewPerspective * vec4(a_Position, 1.0);

    normalModel = a_Normal;
    preNormalWorld = mat3(transpose(inverse(u_Model))) * a_Normal; // not normalized yet

    uv = a_TexCoord;
    color = a_Color;

    gl_Position = positionFrag;
}


#type fragment
#version 460 core

layout(std140) uniform ViewMatrices {
	mat4 u_View;
	mat4 u_Projection;
};

layout(binding = 1, std140) uniform PointLight {
    vec3 u_LightPosition; // = vec3(0.0, 10.0, 0.0);
    vec3 u_LightColor; // = vec3(1.0, 1.0, 1.0);
    vec3 u_LightAttenuation; // = vec3(0.0, 1.0, 0.0);
};

uniform int u_RenderType = 1; // { SolidColor, Normal, UV, Depth }
uniform vec4 u_SolidColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform float u_DepthMax = 5.0;
uniform float u_DepthPow = 2.0;
uniform vec4 u_DiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 u_SkyColor = vec3(0.0, 0.0, 1.0);
uniform vec3 u_GroundColor = vec3(0.0, 1.0, 0.0);

in vec3 positionWorld;
in vec3 positionView;
in vec4 positionFrag;
in vec3 normalModel;
in vec3 preNormalWorld;
in vec2 uv;
in vec4 color;

layout (location = 0) out vec4 outColor;

void main() {
    vec3 normal = normalize(preNormalWorld);
    switch (u_RenderType) {
    case 0: // Solid Color
        outColor = u_SolidColor;
        break;
    case 1: // Normal
        outColor = vec4(normalModel * 0.5 + 0.5, 1.0);
        break;
    case 2: // UV
        outColor = vec4(uv.x, uv.y, 0.0, 1.0);
        break;
    case 3: // Depth
        outColor = vec4(vec3(1.0) - pow(positionFrag.z / u_DepthMax, u_DepthPow), 1.0);
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
        vec3 relLightPos = u_LightPosition - positionWorld;
        float lightDist = length(relLightPos);
        vec3 lightDir = relLightPos / lightDist; // normalize
        float attenuation = 1.0 / (u_LightAttenuation.x + u_LightAttenuation.y * lightDist + u_LightAttenuation.z * lightDist * lightDist);
        float diffuseVal = max(0.0, dot(normal, lightDir));
        vec3 lightScattered = u_LightColor * diffuseVal * attenuation;
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