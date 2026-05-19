#pragma once

#include "physics/Collision.h"
#include "renderer/Math.h"
#include "scene/Entity.h"

#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace plunger {

class Scene;

struct CharacterPhysicsSettings {
    // Sized to match the Character body parts (torso width, head top).
    float halfWidth = 0.29f;
    float height = 1.58f;
    float gravity = -18.f;
    float jumpVelocity = 10.f;
    float groundSnapEpsilon = 0.02f;
};

struct CharacterPhysicsState {
    float verticalVelocity = 0.f;
    bool onGround = true;
    Vec3 horizontalVelocity = {0.f, 0.f, 0.f};
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
    void resolveCharacterCollisions(Vec3& feetPosition, const CharacterPhysicsSettings& settings) const;
    void snapToGround(Vec3& feetPosition, const CharacterPhysicsSettings& settings) const;
    bool probeGround(const Vec3& feetPosition, const CharacterPhysicsSettings& settings) const;

    struct DynamicAabb {
        EntityId id;
        Aabb aabb;
        Vec3 velocity = {0.f, 0.f, 0.f};
    };

    std::vector<Aabb> m_staticAabbs;
    std::vector<DynamicAabb> m_dynamicAabbs;
    std::vector<SphereCollider> m_spheres;
    std::unordered_map<EntityId, Aabb> m_previousDynamicAabbs;
    float m_lastRebuildTime = -1.f;
};

} // namespace plunger
