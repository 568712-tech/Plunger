#pragma once

#include "renderer/Math.h"
#include "scene/Entity.h"

#include <vector>

namespace plunger {

struct Transform {
    Vec3 position {0.f, 0.f, 0.f};
    Vec3 rotation {0.f, 0.f, 0.f};
    Vec3 scaleFactor {1.f, 1.f, 1.f};
    EntityId parent = InvalidEntity;
    std::vector<EntityId> children;
    Mat4 localMatrix = Mat4::identity();
    Mat4 worldMatrix = Mat4::identity();
    bool dirty = true;

    void rebuildLocalMatrix();
};

} // namespace plunger