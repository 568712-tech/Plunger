#pragma once

#include "renderer/Math.h"

#include <algorithm>
#include <cmath>

namespace plunger {

struct Aabb {
    Vec3 min;
    Vec3 max;
};

struct SphereCollider {
    Vec3 center;
    float radius = 0.f;
};

inline bool aabbOverlaps(const Aabb& a, const Aabb& b)
{
    return a.min.x <= b.max.x && a.max.x >= b.min.x
        && a.min.y <= b.max.y && a.max.y >= b.min.y
        && a.min.z <= b.max.z && a.max.z >= b.min.z;
}

inline Aabb makeCharacterAabb(const Vec3& feetPosition, float halfWidth, float height)
{
    return {
        {feetPosition.x - halfWidth, feetPosition.y, feetPosition.z - halfWidth},
        {feetPosition.x + halfWidth, feetPosition.y + height, feetPosition.z + halfWidth},
    };
}

inline bool sphereOverlapsAabb(const SphereCollider& sphere, const Aabb& box)
{
    const float closestX = std::clamp(sphere.center.x, box.min.x, box.max.x);
    const float closestY = std::clamp(sphere.center.y, box.min.y, box.max.y);
    const float closestZ = std::clamp(sphere.center.z, box.min.z, box.max.z);
    const Vec3 closest {closestX, closestY, closestZ};
    const Vec3 delta = sphere.center - closest;
    return dot(delta, delta) <= sphere.radius * sphere.radius;
}

inline bool sphereOverlapsSphere(const SphereCollider& a, const SphereCollider& b)
{
    const Vec3 delta = a.center - b.center;
    const float combinedRadius = a.radius + b.radius;
    return dot(delta, delta) <= combinedRadius * combinedRadius;
}

inline void resolveAabbPenetration(Vec3& position, const Aabb& character, const Aabb& obstacle)
{
    const float overlapX = std::min(character.max.x - obstacle.min.x, obstacle.max.x - character.min.x);
    const float overlapY = std::min(character.max.y - obstacle.min.y, obstacle.max.y - character.min.y);
    const float overlapZ = std::min(character.max.z - obstacle.min.z, obstacle.max.z - character.min.z);

    if (overlapX <= 0.f || overlapY <= 0.f || overlapZ <= 0.f) {
        return;
    }

    if (overlapX <= overlapY && overlapX <= overlapZ) {
        const float characterCenterX = (character.min.x + character.max.x) * 0.5f;
        const float obstacleCenterX = (obstacle.min.x + obstacle.max.x) * 0.5f;
        position.x += (characterCenterX < obstacleCenterX) ? -overlapX : overlapX;
        return;
    }

    if (overlapY <= overlapZ) {
        const float characterCenterY = (character.min.y + character.max.y) * 0.5f;
        const float obstacleCenterY = (obstacle.min.y + obstacle.max.y) * 0.5f;
        position.y += (characterCenterY < obstacleCenterY) ? -overlapY : overlapY;
        return;
    }

    const float characterCenterZ = (character.min.z + character.max.z) * 0.5f;
    const float obstacleCenterZ = (obstacle.min.z + obstacle.max.z) * 0.5f;
    position.z += (characterCenterZ < obstacleCenterZ) ? -overlapZ : overlapZ;
}

inline void resolveSpherePenetration(Vec3& position, const Aabb& character, const SphereCollider& sphere)
{
    const float closestX = std::clamp(sphere.center.x, character.min.x, character.max.x);
    const float closestY = std::clamp(sphere.center.y, character.min.y, character.max.y);
    const float closestZ = std::clamp(sphere.center.z, character.min.z, character.max.z);
    Vec3 closest {closestX, closestY, closestZ};

    Vec3 delta = closest - sphere.center;
    float distance = length(delta);
    if (distance < 1e-5f) {
        delta = {0.f, 1.f, 0.f};
        distance = 1.f;
    }

    const float penetration = sphere.radius - distance;
    if (penetration <= 0.f) {
        return;
    }

    const Vec3 correction = normalize(delta) * penetration;
    position.x += correction.x;
    position.y += correction.y;
    position.z += correction.z;
}

} // namespace plunger
