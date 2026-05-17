#pragma once

#include "renderer/Mesh.h"
#include "renderer/Texture.h"
#include "scene/Entity.h"
#include "scene/Transform.h"

#include <string>

namespace plunger {

enum class PartShape {
    Box,
    Cylinder,
    Pyramid,
    Wedge,
    Cone,
    Sphere,
    Hemisphere,
    Torus,
    TriPrism,
    HexPrism,
    Octahedron,
    Capsule,
    Count,
};

struct Material {
    Vec3 baseColor {1.f, 1.f, 1.f};
    float roughness = 1.f;
    float metallic = 0.f;
    std::string texturePath;
};

struct MeshRenderer {
    const Mesh* mesh = nullptr;
    const Texture* texture = nullptr;
    Material material;
};

struct Part {
    Vec3 position {0.f, 0.f, 0.f};
    Vec3 size {1.f, 1.f, 1.f};
    Vec3 rotation {0.f, 0.f, 0.f};
    float bobAmplitude = 0.f;
    PartShape shape = PartShape::Box;
    Material material;
    
    // Complex movement parameters
    float circleRadius = 0.f;  // 0 = no circular motion
    float circleSpeed = 1.0f;   // radians per second
    float circleAxisY = true;   // rotate around Y axis (true) or X axis (false)
    float swayAmplitude = 0.f;  // side-to-side motion amplitude
    float swaySpeed = 1.0f;     // side-to-side motion speed
    float spinSpeed = 0.f;      // rotation speed (radians per second)
};

} // namespace plunger
