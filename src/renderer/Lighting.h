#pragma once

#include "renderer/Math.h"
#include "renderer/Shader.h"

namespace plunger {

struct PointLight {
    Vec3 position {0.f, 0.f, 0.f};
    Vec3 color {1.f, 1.f, 1.f};
    float intensity = 1.f;
    float radius = 10.f;
};

struct LightingEnvironment {
    Vec3 lightDirection {-0.4f, -1.f, -0.25f};
    Vec3 ambientSky {0.22f, 0.27f, 0.38f};
    Vec3 ambientGround {0.12f, 0.09f, 0.07f};
    Vec3 fogColor {0.08f, 0.10f, 0.14f};
    float fogNear = 10.f;
    float fogFar = 40.f;
    int pointLightCount = 0;
    PointLight pointLights[8] {};

    static LightingEnvironment createDemo();
    void apply(const Shader& shader) const;
};

} // namespace plunger
