#pragma once

#include "renderer/Mesh.h"
#include "renderer/Texture.h"
#include "scene/Entity.h"
#include "scene/Transform.h"

#include <string>

namespace plunger {

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
    Material material;
};

} // namespace plunger