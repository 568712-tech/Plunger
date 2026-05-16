#version 330 core

in vec3 vNormal;
in vec3 vColor;

uniform vec3 uLightDirection;

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(vNormal);
    float diffuse = max(dot(normal, normalize(-uLightDirection)), 0.0);
    vec3 ambient = vColor * 0.30;
    vec3 lit = vColor * (0.70 * diffuse);
    FragColor = vec4(ambient + lit, 1.0);
}