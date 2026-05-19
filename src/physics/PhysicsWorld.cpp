#include "physics/PhysicsWorld.h"

#include "scene/Components.h"
#include "scene/PartMotion.h"
#include "scene/Scene.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace plunger {

// Debug logging for physics. Set to true only when actively diagnosing.
static bool s_debugPhysics = false;


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
    const float dt = (m_lastRebuildTime < 0.f) ? 0.f : (timeSeconds - m_lastRebuildTime);
    scene.forEachPart([&](const Entity& entity, const Part& part) {
        if (excludedEntities.find(entity.id) != excludedEntities.end()) {
            ++partIndex;
            return;
        }

        if (part.size.x <= 0.01f || part.size.y <= 0.01f || part.size.z <= 0.01f) {
            ++partIndex;
            return;
        }

        // Decorative shapes use a loose axis-aligned bounds that leaves large invisible gaps.
        if (part.shape == PartShape::Torus || part.shape == PartShape::Octahedron) {
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
            Aabb currentAabb{minCorner, maxCorner};

            Vec3 velocity{0.f, 0.f, 0.f};
            auto it = m_previousDynamicAabbs.find(entity.id);
            if (it != m_previousDynamicAabbs.end() && dt > 1e-6f) {
                const Aabb& prev = it->second;
                const Vec3 prevCenter = {(prev.min.x + prev.max.x) * 0.5f, (prev.min.y + prev.max.y) * 0.5f, (prev.min.z + prev.max.z) * 0.5f};
                const Vec3 curCenter = {(currentAabb.min.x + currentAabb.max.x) * 0.5f, (currentAabb.min.y + currentAabb.max.y) * 0.5f, (currentAabb.min.z + currentAabb.max.z) * 0.5f};
                velocity = {(curCenter.x - prevCenter.x) / dt, (curCenter.y - prevCenter.y) / dt, (curCenter.z - prevCenter.z) / dt};
            }

            m_dynamicAabbs.push_back({entity.id, currentAabb, velocity});

            // store current for next frame
            m_previousDynamicAabbs[entity.id] = currentAabb;
        }

        ++partIndex;
    });

    m_lastRebuildTime = timeSeconds;
}

void PhysicsWorld::addStaticAabb(const Aabb& aabb)
{
    m_staticAabbs.push_back(aabb);
}

void PhysicsWorld::resolveCharacterCollisions(Vec3& feetPosition, const CharacterPhysicsSettings& settings) const
{
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
        for (const DynamicAabb& d : m_dynamicAabbs) {
            const Aabb& collider = d.aabb;
            const Aabb currentCharacter = makeCharacterAabb(feetPosition, settings.halfWidth, settings.height);
            if (!aabbOverlaps(currentCharacter, collider)) {
                continue;
            }

            resolveAabbPenetration(feetPosition, currentCharacter, collider);
        }

        for (const SphereCollider& sphere : m_spheres) {
            const Aabb updatedCharacter = makeCharacterAabb(feetPosition, settings.halfWidth, settings.height);
            if (!sphereOverlapsAabb(sphere, updatedCharacter)) {
                continue;
            }

            resolveSpherePenetration(feetPosition, updatedCharacter, sphere);
        }
    }
}

void PhysicsWorld::snapToGround(Vec3& feetPosition, const CharacterPhysicsSettings& settings) const
{
    const Aabb character = makeCharacterAabb(feetPosition, settings.halfWidth, settings.height);
    float highestSurface = -1e9f;

    auto considerCollider = [&](const Aabb& collider) {
        if (character.min.x >= collider.max.x || character.max.x <= collider.min.x) {
            return;
        }
        if (character.min.z >= collider.max.z || character.max.z <= collider.min.z) {
            return;
        }
        if (collider.max.y > feetPosition.y + settings.groundSnapEpsilon) {
            return;
        }
        if (collider.max.y < feetPosition.y - settings.groundSnapEpsilon) {
            return;
        }

        highestSurface = std::max(highestSurface, collider.max.y);
    };

    for (const Aabb& collider : m_staticAabbs) {
        considerCollider(collider);
    }
    for (const DynamicAabb& d : m_dynamicAabbs) {
        considerCollider(d.aabb);
    }

    if (highestSurface > -1e8f) {
        feetPosition.y = highestSurface;
    }
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

    if (s_debugPhysics) {
        std::cout << "PhysicsWorld: probing ground at feetPosition=(" << feetPosition.x << "," << feetPosition.y << "," << feetPosition.z << ")\n";

        for (const Aabb& collider : m_staticAabbs) {
            if (testAabbGround(collider)) {
                std::cout << "probeGround: matched static collider minY=" << collider.min.y << " maxY=" << collider.max.y << '\n';
                return true;
            }
        }

        // Debug: print colliders for troubleshooting invisible raises (only when enabled)
        std::cout << "PhysicsWorld: staticAabbs=" << m_staticAabbs.size()
              << " dynamicAabbs=" << m_dynamicAabbs.size()
                  << " spheres=" << m_spheres.size() << '\n';
        for (std::size_t i = 0; i < m_staticAabbs.size(); ++i) {
            const Aabb& a = m_staticAabbs[i];
            std::cout << " static[" << i << "] minY=" << a.min.y << " maxY=" << a.max.y
                      << " minX=" << a.min.x << " maxX=" << a.max.x
                      << " minZ=" << a.min.z << " maxZ=" << a.max.z << '\n';
        }
        for (std::size_t i = 0; i < m_dynamicAabbs.size(); ++i) {
            const Aabb& a = m_dynamicAabbs[i].aabb;
            std::cout << " dynamic[" << i << "] minY=" << a.min.y << " maxY=" << a.max.y
                      << " minX=" << a.min.x << " maxX=" << a.max.x
                      << " minZ=" << a.min.z << " maxZ=" << a.max.z << '\n';
        }

        for (const DynamicAabb& d : m_dynamicAabbs) {
            const Aabb& collider = d.aabb;
            if (testAabbGround(collider)) {
                std::cout << "probeGround: matched dynamic collider minY=" << collider.min.y << " maxY=" << collider.max.y << '\n';
                return true;
            }
        }
    } else {
        for (const Aabb& collider : m_staticAabbs) {
            if (testAabbGround(collider)) {
                return true;
            }
        }
        for (const DynamicAabb& d : m_dynamicAabbs) {
            if (testAabbGround(d.aabb)) {
                return true;
            }
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
    // Determine platform velocity under the character (if any)
    Vec3 platformVelocity{0.f, 0.f, 0.f};
    if (state.onGround) {
        const Aabb character = makeCharacterAabb(feetPosition, settings.halfWidth, settings.height);
        const float probeMinY = feetPosition.y - settings.groundSnapEpsilon;
        const float probeMaxY = feetPosition.y + settings.groundSnapEpsilon;

        for (const DynamicAabb& d : m_dynamicAabbs) {
            const Aabb& collider = d.aabb;
            if (collider.max.y < probeMinY || collider.max.y > probeMaxY) {
                continue;
            }
            if (character.min.x <= collider.max.x && character.max.x >= collider.min.x
                && character.min.z <= collider.max.z && character.max.z >= collider.min.z) {
                platformVelocity = d.velocity;
                break;
            }
        }
    }

    // Compute input velocity from provided delta
    Vec3 inputVelocity{0.f, 0.f, 0.f};
    if (deltaTime > 1e-6f) {
        inputVelocity.x = horizontalDelta.x / deltaTime;
        inputVelocity.z = horizontalDelta.z / deltaTime;
    }

    if (state.onGround) {
        // On ground: player's horizontal velocity is platform + input
        state.horizontalVelocity.x = platformVelocity.x + inputVelocity.x;
        state.horizontalVelocity.z = platformVelocity.z + inputVelocity.z;
    } else {
        // In air: allow limited steering (air control) towards input velocity, but only if there's input
        const float inputLen = std::sqrt(inputVelocity.x * inputVelocity.x + inputVelocity.z * inputVelocity.z);
        if (inputLen > 1e-6f) {
            const float airAccel = 12.0f; // units per second^2 (tunable)
            const float maxChange = airAccel * deltaTime;

            float diffX = inputVelocity.x - state.horizontalVelocity.x;
            float diffZ = inputVelocity.z - state.horizontalVelocity.z;

            if (diffX > maxChange) diffX = maxChange;
            if (diffX < -maxChange) diffX = -maxChange;
            if (diffZ > maxChange) diffZ = maxChange;
            if (diffZ < -maxChange) diffZ = -maxChange;

            state.horizontalVelocity.x += diffX;
            state.horizontalVelocity.z += diffZ;
        }
        // Else: no input, maintain current velocity (preserves platform momentum)
    }

    if (jumpRequested && state.onGround) {
        state.verticalVelocity = settings.jumpVelocity;
        state.onGround = false;
        // keep horizontalVelocity as set (inherits platform velocity)
    }

    state.verticalVelocity += settings.gravity * deltaTime;

    // Move and resolve one axis at a time so the capsule stops flush with surfaces
    // instead of up to one frame's movement short of contact.
    feetPosition.x += state.horizontalVelocity.x * deltaTime;
    resolveCharacterCollisions(feetPosition, settings);

    feetPosition.z += state.horizontalVelocity.z * deltaTime;
    resolveCharacterCollisions(feetPosition, settings);

    const float yBeforeVerticalMove = feetPosition.y;
    const float intendedYDelta = state.verticalVelocity * deltaTime;
    feetPosition.y += intendedYDelta;
    resolveCharacterCollisions(feetPosition, settings);

    // Head bump detection: if moving upward but collision blocked most of it, zero velocity
    if (intendedYDelta > 0.f) {
        float actualYDelta = feetPosition.y - yBeforeVerticalMove;
        if (actualYDelta < intendedYDelta * 0.2f) {  // blocked more than 80%
            state.verticalVelocity = 0.f;
        }
    }

    snapToGround(feetPosition, settings);

    state.onGround = probeGround(feetPosition, settings);
    if (state.onGround && state.verticalVelocity <= 0.f) {
        state.verticalVelocity = 0.f;
    }
}

} // namespace plunger
