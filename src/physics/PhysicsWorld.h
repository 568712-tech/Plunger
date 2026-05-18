#pragma once

#include "physics/Collision.h"
#include "renderer/Math.h"
#include "scene/Entity.h"

#include <unordered_set>
#include <vector>

namespace plunger {

class Scene;

struct CharacterPhysicsSettings {
    float halfWidth = 0.30f;
    float height = 1.75f;
    float gravity = -18.f;
    float jumpVelocity = 9.f;
    float groundSnapEpsilon = 0.08f;
};

struct CharacterPhysicsState {
    float verticalVelocity = 0.f;
    bool onGround = true;
};

class PhysicsWorld {
public:
    void rebuildFromScene(const Scene& scene,
                          float timeSeconds,
                          const std::unordered_set<EntityId>& excludedEntities);
    void clearStaticColliders();
    void addStaticAabb(const Aabb& aabb);

    void simulateCharacter(Vec3& feetPosition,
                           CharacterPhysicsState& state,
                           const Vec3& horizontalDelta,
                           bool jumpRequested,
                           float deltaTime,
                           const CharacterPhysicsSettings& settings = {}) const;

private:
    bool probeGround(const Vec3& feetPosition, const CharacterPhysicsSettings& settings) const;

    std::vector<Aabb> m_staticAabbs;
    std::vector<Aabb> m_dynamicAabbs;
    std::vector<SphereCollider> m_spheres;
};

} // namespace plunger
