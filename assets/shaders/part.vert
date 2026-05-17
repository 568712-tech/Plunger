#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in mat4 iModel;
layout(location = 7) in vec3 iColor;
layout(location = 8) in float iRoughness;
layout(location = 9) in float iMetallic;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightSpace;

out vec3 vWorldPosition;
out vec3 vNormal;
out vec3 vColor;
out float vRoughness;
out float vMetallic;
out vec4 vLightSpacePosition;

void main()
{
    vec4 worldPosition = iModel * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    vNormal = mat3(transpose(inverse(iModel))) * aNormal;
    vColor = iColor;
    vRoughness = iRoughness;
    vMetallic = iMetallic;
    vLightSpacePosition = uLightSpace * worldPosition;
    gl_Position = uProjection * uView * worldPosition;
}
