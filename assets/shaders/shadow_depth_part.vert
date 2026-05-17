#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 3) in mat4 iModel;

uniform mat4 uLightSpace;

void main()
{
    gl_Position = uLightSpace * iModel * vec4(aPosition, 1.0);
}
