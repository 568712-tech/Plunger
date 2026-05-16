#version 330 core

in vec3 vWorldPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 uLightDirection;
uniform sampler2D uTexture;
uniform vec3 uBaseColor;

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 lightDirection = normalize(-uLightDirection);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    vec3 sampledColor = texture(uTexture, vTexCoord).rgb;
    vec3 baseColor = sampledColor * uBaseColor;
    vec3 ambient = baseColor * 0.28;
    vec3 lit = baseColor * (0.72 * diffuse);
    FragColor = vec4(ambient + lit, 1.0);
}