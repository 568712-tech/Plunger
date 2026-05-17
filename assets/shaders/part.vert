#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in mat4 iModel;
layout(location = 7) in vec3 iColor;

uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightSpace;

out vec3 vWorldPosition;
out vec3 vNormal;
out vec3 vColor;
out vec4 vLightSpacePosition;

void main()
{
    vec4 worldPosition = iModel * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    vNormal = mat3(transpose(inverse(iModel))) * aNormal;
    vColor = iColor;
    vLightSpacePosition = uLightSpace * worldPosition;
    gl_Position = uProjection * uView * worldPosition;
}
