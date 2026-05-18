#include "physics/PhysicsWorld.h"

#include "scene/Components.h"
#include "scene/PartMotion.h"
#include "scene/Scene.h"

#include <algorithm>
#include <cmath>

namespace plunger {

void PhysicsWorld::clearStaticColliders()
{
    m_staticAabbs.clear();
}

void PhysicsWorld::rebuildFromScene(const Scene& scene,
                                    float timeSeconds,
                                    const std::unordered_set<EntityId>& excludedEntities)
{
    m_dynamicAabbs.clear();
    m_spheres.clear();

    std::size_t partIndex = 0;
    scene.forEachPart([&](const Entity& entity, const Part& part) {
        if (excludedEntities.find(entity.id) != excludedEntities.end()) {
            ++partIndex;
            return;
        }

        if (part.size.x <= 0.01f || part.size.y <= 0.01f || part.size.z <= 0.01f) {
            ++partIndex;
            return;
        }

        const Mat4 modelMatrix = composePartModelMatrix(part, timeSeconds, partIndex);

        if (part.shape == PartShape::Sphere || part.shape == PartShape::Hemisphere) {
            const Vec3 center = transformPoint(modelMatrix, {0.f, 0.f, 0.f});
            const Vec3 radiusSample = transformPoint(modelMatrix, {0.5f, 0.f, 0.f}) - center;
            const float radius = length(radiusSample);
            if (radius > 0.01f) {
                m_spheres.push_back({center, radius});
            }
        } else {
            Vec3 minCorner;
            Vec3 maxCorner;
            computeWorldAabb(modelMatrix, minCorner, maxCorner);
            m_dynamicAabbs.push_back({minCorner, maxCorner});
        }

        ++partIndex;
    });
}

void PhysicsWorld::addStaticAabb(const Aabb& aabb)
{
    m_staticAabbs.push_back(aabb);
}

bool PhysicsWorld::probeGround(const Vec3& feetPosition, const CharacterPhysicsSettings& settings) const
{
    const Aabb character = makeCharacterAabb(feetPosition, settings.halfWidth, settings.height);
    const float probeMinY = feetPosition.y - settings.groundSnapEpsilon;
    const float probeMaxY = feetPosition.y + settings.groundSnapEpsilon;

    auto testAabbGround = [&](const Aabb& collider) {
        if (collider.max.y < probeMinY || collider.max.y > probeMaxY) {
            return false;
        }

        return character.min.x <= collider.max.x && character.max.x >= collider.min.x
            && character.min.z <= collider.max.z && character.max.z >= collider.min.z;
    };

    for (const Aabb& collider : m_staticAabbs) {
        if (testAabbGround(collider)) {
            return true;
        }
    }

    for (const Aabb& collider : m_dynamicAabbs) {
        if (testAabbGround(collider)) {
            return true;
        }
    }

    for (const SphereCollider& sphere : m_spheres) {
        if (std::abs(sphere.center.y - sphere.radius - feetPosition.y) > settings.groundSnapEpsilon) {
            continue;
        }

        const float horizontalDistance = std::sqrt(
            (feetPosition.x - sphere.center.x) * (feetPosition.x - sphere.center.x)
            + (feetPosition.z - sphere.center.z) * (feetPosition.z - sphere.center.z));

        if (horizontalDistance <= settings.halfWidth + sphere.radius + 0.05f) {
            return true;
        }
    }

    return false;
}

void PhysicsWorld::simulateCharacter(Vec3& feetPosition,
                                     CharacterPhysicsState& state,
                                     const Vec3& horizontalDelta,
                                     bool jumpRequested,
                                     float deltaTime,
                                     const CharacterPhysicsSettings& settings) const
{
    if (jumpRequested && state.onGround) {
        state.verticalVelocity = settings.jumpVelocity;
        state.onGround = false;
    }

    state.verticalVelocity += settings.gravity * deltaTime;

    feetPosition.x += horizontalDelta.x;
    feetPosition.z += horizontalDelta.z;
    feetPosition.y += state.verticalVelocity * deltaTime;

    constexpr int resolveIterations = 6;
    for (int iteration = 0; iteration < resolveIterations; ++iteration) {
        auto resolveAgainstAabbs = [&](const std::vector<Aabb>& colliders) {
            for (const Aabb& collider : colliders) {
                const Aabb currentCharacter = makeCharacterAabb(feetPosition, settings.halfWidth, settings.height);
                if (!aabbOverlaps(currentCharacter, collider)) {
                    continue;
                }

                resolveAabbPenetration(feetPosition, currentCharacter, collider);
            }
        };

        resolveAgainstAabbs(m_staticAabbs);
        resolveAgainstAabbs(m_dynamicAabbs);

        for (const SphereCollider& sphere : m_spheres) {
            const Aabb updatedCharacter = makeCharacterAabb(feetPosition, settings.halfWidth, settings.height);
            if (!sphereOverlapsAabb(sphere, updatedCharacter)) {
                continue;
            }

            resolveSpherePenetration(feetPosition, updatedCharacter, sphere);
        }
    }

    state.onGround = probeGround(feetPosition, settings);
    if (state.onGround && state.verticalVelocity <= 0.f) {
        state.verticalVelocity = 0.f;
    }
}

} // namespace plunger
