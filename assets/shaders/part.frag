#version 330 core

in vec3 vWorldPosition;
in vec3 vNormal;
in vec3 vColor;
in float vRoughness;
in float vMetallic;
in vec4 vLightSpacePosition;

#define MAX_POINT_LIGHTS 8

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

uniform vec3 uLightDirection;
uniform sampler2D uShadowMap;
uniform vec3 uCameraPosition;
uniform vec3 uAmbientSky;
uniform vec3 uAmbientGround;
uniform vec3 uFogColor;
uniform float uFogNear;
uniform float uFogFar;
uniform int uPointLightCount;
uniform PointLight uPointLights[MAX_POINT_LIGHTS];

out vec4 FragColor;

float shadowFactor(vec3 normal, vec3 sunDirection)
{
    vec3 projected = vLightSpacePosition.xyz / max(vLightSpacePosition.w, 1e-5);
    projected = projected * 0.5 + 0.5;

    if (projected.z > 1.0 || projected.x < 0.0 || projected.x > 1.0 || projected.y < 0.0 || projected.y > 1.0) {
        return 0.0;
    }

    float currentDepth = projected.z;
    // Adaptive bias based on light angle: steeper angles need more bias
    float bias = max(0.002 * (1.0 - dot(normal, sunDirection)), 0.0005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));

    // PCF with 3x3 kernel
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(uShadowMap, projected.xy + vec2(float(x), float(y)) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

vec3 toneMapAces(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 viewDirection = normalize(uCameraPosition - vWorldPosition);
    vec3 sunDirection = normalize(-uLightDirection);

    float ndotUp = normal.y * 0.5 + 0.5;
    vec3 ambientLight = mix(uAmbientGround, uAmbientSky, clamp(ndotUp, 0.0, 1.0));
    vec3 color = vColor * ambientLight;

    float roughness = clamp(vRoughness, 0.18, 1.0);
    float metallic = clamp(vMetallic, 0.0, 1.0);
    float specPower = mix(48.0, 8.0, roughness);
    float specScale = mix(0.04, 0.35, metallic) * mix(1.0, 0.55, 1.0 - roughness);

    float sunDiffuse = max(dot(normal, sunDirection), 0.0);
    float shadow = shadowFactor(normal, sunDirection);
    vec3 sunHalf = normalize(sunDirection + viewDirection);
    float sunSpecular = pow(max(dot(normal, sunHalf), 0.0), specPower) * specScale;
    float sunVisibility = 1.0 - shadow;
    color += vColor * (0.9 * sunDiffuse * sunVisibility);
    color += vec3(1.0, 0.98, 0.95) * (0.3 * sunSpecular * sunVisibility);

    int clampedCount = min(uPointLightCount, MAX_POINT_LIGHTS);
    for (int index = 0; index < clampedCount; ++index) {
        vec3 toLight = uPointLights[index].position - vWorldPosition;
        float distanceToLight = length(toLight);
        if (distanceToLight > uPointLights[index].radius) {
            continue;
        }

        vec3 lightDirection = toLight / max(distanceToLight, 1e-4);
        float attenuation = 1.0 - smoothstep(0.0, uPointLights[index].radius, distanceToLight);
        float diffuse = max(dot(normal, lightDirection), 0.0);
        vec3 halfVector = normalize(lightDirection + viewDirection);
        float specular = pow(max(dot(normal, halfVector), 0.0), specPower) * specScale;

        vec3 lightColor = uPointLights[index].color * min(uPointLights[index].intensity * attenuation, 1.2);
        color += vColor * lightColor * diffuse;
        color += lightColor * (0.12 * specular);
    }

    float fogFactor = smoothstep(uFogNear, uFogFar, length(uCameraPosition - vWorldPosition));
    color = mix(color, uFogColor, fogFactor);
    color = toneMapAces(color);
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
