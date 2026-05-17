#pragma once

#include "renderer/Math.h"
#include "scene/Scene.h"
#include "input/Input.h"
#include "renderer/Camera.h"

#include <vector>

namespace plunger {

class Character {
public:
    Character(Scene& scene, const Vec3& spawnPosition, float spawnYawRadians = 0.f);
    void update(float deltaTime, float timeSeconds, const Input& input, const Camera& camera);
    void setPosition(const Vec3& pos) { m_position = pos; }
    Vec3  position() const { return m_position; }
    float yaw()      const { return m_bodyYaw; }
    void setVisible(bool visible);

private:
    struct PartRecord {
        EntityId id;
        Vec3     offset;
        Vec3     originalSize;
        bool     isHead = false;
    };

    Scene& m_scene;
    Vec3   m_position;

    float m_bodyYaw          = 0.f;
    float m_headYaw          = 0.f;
    float m_smoothWalkSpeed  = 0.f;
    float m_animPhase        = 0.f;

    float m_verticalVelocity = 0.f;
    bool  m_onGround         = true;
    bool  m_wasRightMouseDown = false;
    bool  m_armAnimating     = false;
    float m_armAnimTime      = 0.f;

    std::vector<PartRecord> m_parts;
};

}