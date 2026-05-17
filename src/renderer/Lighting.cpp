#include "renderer/Lighting.h"

#include <string>

namespace plunger {

LightingEnvironment LightingEnvironment::createDemo()
{
    LightingEnvironment lighting;
    lighting.lightDirection = {-0.4f, -1.f, -0.25f};
    lighting.pointLightCount = 3;
    lighting.pointLights[0] = {{0.f, 4.2f, -5.0f}, {1.00f, 0.93f, 0.80f}, 2.4f, 12.0f};
    lighting.pointLights[1] = {{-6.0f, 3.0f, 5.0f}, {0.50f, 0.72f, 1.00f}, 1.8f, 10.0f};
    lighting.pointLights[2] = {{6.0f, 3.0f, 5.0f}, {1.00f, 0.62f, 0.45f}, 1.8f, 10.0f};
    return lighting;
}

void LightingEnvironment::apply(const Shader& shader) const
{
    shader.setVec3("uLightDirection", normalize(lightDirection));
    shader.setVec3("uAmbientSky", ambientSky);
    shader.setVec3("uAmbientGround", ambientGround);
    shader.setVec3("uFogColor", fogColor);
    shader.setFloat("uFogNear", fogNear);
    shader.setFloat("uFogFar", fogFar);
    shader.setInt("uPointLightCount", pointLightCount);

    for (int index = 0; index < pointLightCount && index < 8; ++index) {
        const std::string prefix = "uPointLights[" + std::to_string(index) + "]";
        shader.setVec3((prefix + ".position").c_str(), pointLights[index].position);
        shader.setVec3((prefix + ".color").c_str(), pointLights[index].color);
        shader.setFloat((prefix + ".intensity").c_str(), pointLights[index].intensity);
        shader.setFloat((prefix + ".radius").c_str(), pointLights[index].radius);
    }
}

} // namespace plunger
