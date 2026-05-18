#include "game/Character.h"

#include "scene/Components.h"
#include <SFML/Window/Mouse.hpp>

#include <cmath>

namespace plunger {

namespace {
constexpr float PI            = 3.14159265f;
constexpr float baseSpeed     = 5.0f;
constexpr float runMultiplier = 1.8f;

float smoothLerp(float a, float b, float stiffness, float dt)
{
    return a + (b - a) * (1.f - std::pow(stiffness, dt));
}

float smoothAngle(float a, float b, float stiffness, float dt)
{
    float d = b - a;

    while (d >  PI) d -= 2.f * PI;
    while (d < -PI) d += 2.f * PI;

    return a + d * (1.f - std::pow(stiffness, dt));
}
}

static Vec3 rotateYaw(const Vec3& v, float yaw)
{
    const float c = std::cos(yaw);
    const float s = std::sin(yaw);

    return {
        c * v.x + s * v.z,
        v.y,
        -s * v.x + c * v.z
    };
}

Character::Character(Scene& scene,
                     const Vec3& spawnPosition,
                     float spawnYawRadians)
    : m_scene(scene)
    , m_position(spawnPosition)
    , m_bodyYaw(spawnYawRadians)
    , m_headYaw(spawnYawRadians)
{
    auto addPart = [&](const Vec3& offset,
                       const Vec3& size,
                       const Vec3& color,
                       bool isHead = false)
    {
        Part part;

        part.position           = offset;
        part.size               = size;

        part.shape = PartShape::Box;

        part.material.baseColor = color;

        const EntityId id = m_scene.createEntity();

        m_scene.addPart(id, part);

        m_parts.push_back({
            id,
            offset,
            size,
            isHead
        });
    };

    const Vec3 skin      = {0.88f, 0.74f, 0.62f};

    const Vec3 torsoBlue = {0.12f, 0.24f, 0.75f};

    const Vec3 red       = {0.78f, 0.12f, 0.12f};

    const Vec3 pantsBlue = {0.08f, 0.10f, 0.35f};

    addPart(
        {0.f, 0.88f, 0.f},
        {0.58f, 0.68f, 0.30f},
        torsoBlue
    );

    addPart(
        {0.f, 1.40f, 0.f},
        {0.36f, 0.36f, 0.36f},
        skin,
        true
    );

    addPart(
        {-0.40f, 0.88f, 0.f},
        {0.24f, 0.60f, 0.24f},
        red
    );

    addPart(
        {0.40f, 0.88f, 0.f},
        {0.24f, 0.60f, 0.24f},
        red
    );

    addPart(
        {-0.16f, 0.28f, 0.f},
        {0.25f, 0.58f, 0.25f},
        pantsBlue
    );

    addPart(
        {0.16f, 0.28f, 0.f},
        {0.25f, 0.58f, 0.25f},
        pantsBlue
    );
}

void Character::setPosition(const Vec3& pos)
{
    m_position = pos;
    m_physicsState.verticalVelocity = 0.f;
    m_physicsState.onGround = true;
}

void Character::collectExcludedEntities(std::unordered_set<EntityId>& excluded) const
{
    for (const PartRecord& partRecord : m_parts) {
        excluded.insert(partRecord.id);
    }
}

void Character::update(float deltaTime,
                       float timeSeconds,
                       const Input& input,
                       const Camera& camera,
                       const PhysicsWorld& physics)
{
    (void)timeSeconds;

    Vec2 moveInput{0.f, 0.f};

    if (input.isKeyDown(sf::Keyboard::Key::W))
        moveInput.y += 1.f;

    if (input.isKeyDown(sf::Keyboard::Key::S))
        moveInput.y -= 1.f;

    if (input.isKeyDown(sf::Keyboard::Key::A))
        moveInput.x -= 1.f;

    if (input.isKeyDown(sf::Keyboard::Key::D))
        moveInput.x += 1.f;

    const float inputLen =
        std::sqrt(moveInput.x * moveInput.x +
                  moveInput.y * moveInput.y);

    const bool moving = inputLen > 0.001f;

    Vec3 moveDir{0.f, 0.f, 0.f};

    Vec3 camFwd = camera.forward();

    camFwd.y = 0.f;

    const float cfLen =
        std::sqrt(camFwd.x * camFwd.x +
                  camFwd.z * camFwd.z);

    if (cfLen > 1e-5f) {
        camFwd.x /= cfLen;
        camFwd.z /= cfLen;
    }
    else {
        camFwd = {0.f, 0.f, -1.f};
    }

    m_headYaw = std::atan2(camFwd.x, camFwd.z);

    if (moving) {

        moveInput.x /= inputLen;
        moveInput.y /= inputLen;

        const Vec3 camRight =
            normalize(cross(camFwd, {0.f, 1.f, 0.f}));

        moveDir = normalize({
            camRight.x * moveInput.x +
            camFwd.x * moveInput.y,

            0.f,

            camRight.z * moveInput.x +
            camFwd.z * moveInput.y
        });

        if (moveInput.y >= 0.f) {

            const float targetBodyYaw =
                std::atan2(moveDir.x, moveDir.z);

            m_bodyYaw = smoothAngle(
                m_bodyYaw,
                targetBodyYaw,
                0.008f,
                deltaTime
            );
        }
    }

    const bool running =
        input.isKeyDown(sf::Keyboard::Key::LShift) ||
        input.isKeyDown(sf::Keyboard::Key::RShift);

    const float speed =
        baseSpeed * (running ? runMultiplier : 1.f);

    const Vec3 horizontalDelta {
        moving ? moveDir.x * speed * deltaTime : 0.f,
        0.f,
        moving ? moveDir.z * speed * deltaTime : 0.f,
    };

    const bool jumpRequested =
        input.isKeyDown(sf::Keyboard::Key::Space) && m_physicsState.onGround;

    physics.simulateCharacter(
        m_position,
        m_physicsState,
        horizontalDelta,
        jumpRequested,
        deltaTime);

    const bool jumping = !m_physicsState.onGround;

    const bool rightDown =
        sf::Mouse::isButtonPressed(
            sf::Mouse::Button::Right
        );

    if (rightDown && !m_wasRightMouseDown && !m_armAnimating) {
        m_armAnimating = true;
        m_armAnimTime = 0.f;
    }
    m_wasRightMouseDown = rightDown;

    float armPosition = 0.f;
    if (m_armAnimating) {
        m_armAnimTime += deltaTime;
        const float animDuration = 0.15f;
        if (m_armAnimTime >= animDuration) {
            m_armAnimating = false;
            armPosition = 0.f;
        } else {
            const float t = m_armAnimTime / animDuration;
            armPosition = std::sin(t * PI);
        }
    }

    const float targetWalk =
        moving
            ? (running ? 1.0f : 0.60f)
            : 0.f;

    m_smoothWalkSpeed =
        smoothLerp(
            m_smoothWalkSpeed,
            targetWalk,
            0.001f,
            deltaTime
        );

    const float cadence =
        running ? 10.5f : 7.0f;

    m_animPhase +=
        deltaTime *
        cadence *
        (moving ? 1.f : 0.25f);

    const float sw = m_smoothWalkSpeed;

    for (size_t i = 0; i < m_parts.size(); ++i) {

        Part* part =
            m_scene.getPart(m_parts[i].id);

        if (!part)
            continue;

        const float yaw =
            m_parts[i].isHead
                ? m_headYaw
                : m_bodyYaw;

        const Vec3 rot =
            rotateYaw(
                m_parts[i].offset,
                yaw
            );

        part->position = {
            m_position.x + rot.x,
            m_position.y + rot.y,
            m_position.z + rot.z
        };

        part->rotation = {
            0.f,
            yaw,
            0.f
        };

        if (i == 3) {

            float swing =
                std::sin(m_animPhase + PI)
                * 0.35f
                * sw;

            if (jumping)
                swing = -0.45f;

            part->rotation.x = swing;
            part->rotation.z = 0.f;
        }
        if (i == 2) {
            float swing = std::sin(m_animPhase) * 0.35f * sw;

            const float MAX_SWING = 0.65f;
            swing = std::clamp(swing, -MAX_SWING, MAX_SWING);

            if (jumping)
                swing = -0.35f;

            const float upSwing = -0.85f;
            const float downSwing = swing;
            const float blendedSwing = smoothLerp(downSwing, upSwing, 0.001f, armPosition);

            part->rotation.x = blendedSwing;
            part->rotation.z = -0.08f * armPosition;

        }

        if (i == 4) {

            part->rotation.x =
                jumping
                    ? 0.40f
                    : std::sin(m_animPhase)
                        * 0.65f
                        * sw;
        }

        if (i == 5) {

            part->rotation.x =
                jumping
                    ? -0.40f
                    : std::sin(m_animPhase + PI)
                        * 0.65f
                        * sw;
        }

        part->bobAmplitude =
            (i == 0 || i == 1)
                ? sw * (running ? 0.08f : 0.04f)
                : 0.f;
    }
}

void Character::setVisible(bool visible)
{
    for (const auto& partRecord : m_parts) {
        Part* part = m_scene.getPart(partRecord.id);
        if (part) {
            part->size = visible ? partRecord.originalSize : Vec3{0.f, 0.f, 0.f};
        }
    }
}

}