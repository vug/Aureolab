#type vertex
#version 460 core

layout(std140) uniform ViewData {
	mat4 u_View;
	mat4 u_Projection;
    vec4 u_ViewPositionWorld;
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

layout(std140) uniform ViewData {
	mat4 u_View;
	mat4 u_Projection;
    vec4 u_ViewPositionWorld;
};

#define MAX_LIGHTS 10
struct PointLight {
    vec4 attenuation; // vec3
    vec4 position; // vec3
};
struct DirectionalLight {
    vec4 direction; // vec3
};
struct Light {
    vec4 color;
    PointLight pointParams;
    DirectionalLight directionalParams;
    float intensity;
    int type;
};
layout(std140) uniform Lights {
    Light lights[MAX_LIGHTS];
    vec4 ambientLight;
    int numLights;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float alpha;
};

uniform int u_RenderType = 1; // { SolidColor, Normal, UV, Depth }
uniform vec4 u_SolidColor = vec4(1.0, 1.0, 1.0, 1.0);
uniform float u_DepthMax = 5.0;
uniform float u_DepthPow = 2.0;
uniform Material u_Material;
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
    case 7: // Lit
        vec3 diffuseLight = vec3(0.0f);
        vec3 specularLight = vec3(0.0f);
        for (int i = 0; i < numLights; i++) {
            vec3 lightDir;
            Light light = lights[i];
            switch (light.type) {
                case 0: { // PointLight
                    vec3 relLightPos = light.pointParams.position.xyz - positionWorld;
                    float lightDist = length(relLightPos);
                    lightDir = relLightPos / lightDist; // normalize
                    vec3 att = light.pointParams.attenuation.xyz;
                    float attFactor = 1.0 / (att.x + att.y * lightDist + att.z * lightDist * lightDist);
                    float diffuseVal = max(0.0, dot(normal, lightDir));
                    diffuseLight += light.intensity * light.color.rgb * diffuseVal * attFactor;
                }
                break;
                case 1: { // DirectionalLight
                    lightDir = -normalize(light.directionalParams.direction.xyz);
                    float diffuseVal = max(0.0, dot(normal, lightDir));
                    diffuseLight += light.intensity * light.color.rgb * diffuseVal;
                }
                break;
            }

            // specular
            vec3 viewDir = normalize(u_ViewPositionWorld.xyz - positionWorld);
            vec3 reflectDir = reflect(-lightDir, normal);  
            float specularVal = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
            specularLight += light.intensity * light.color.rgb * specularVal;  
        }
        vec3 rgb = ambientLight.rgb * u_Material.ambient + diffuseLight * u_Material.diffuse + specularLight * u_Material.specular;
        outColor = vec4(rgb, u_Material.alpha);
        break;
    case 8: // Hemispherical Light
        float costheta = dot(normal, vec3(0.0, 1.0, 0.0));
        float a = costheta * 0.5 + 0.5;
        outColor = vec4(mix(u_GroundColor, u_SkyColor, a), 1.0);
        break;
    }
}