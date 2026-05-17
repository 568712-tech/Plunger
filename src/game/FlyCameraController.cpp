#include "game/FlyCameraController.h"

#include "renderer/Math.h"

#include <SFML/Window/Keyboard.hpp>

namespace plunger {

void FlyCameraController::update(Camera& camera, const Input& input, float deltaTime)
{
    if (input.mouseCaptured()) {
        const Vec2 mouseDelta = input.mouseDelta();
        camera.addYaw(mouseDelta.x * mouseSensitivity);
        camera.addPitch(-mouseDelta.y * mouseSensitivity);
    }

    const float speed = moveSpeed * deltaTime;

    Vec3 movement {0.f, 0.f, 0.f};
    Vec3 forward = camera.forward();
    forward.y = 0.f;
    forward = normalize(forward);
    const Vec3 right = normalize(cross(forward, {0.f, 1.f, 0.f}));
    const Vec3 up {0.f, 1.f, 0.f};

    if (input.isKeyDown(sf::Keyboard::Key::W)) {
        movement = movement + forward;
    }
    if (input.isKeyDown(sf::Keyboard::Key::S)) {
        movement = movement - forward;
    }
    if (input.isKeyDown(sf::Keyboard::Key::D)) {
        movement = movement + right;
    }
    if (input.isKeyDown(sf::Keyboard::Key::A)) {
        movement = movement - right;
    }
    if (input.isKeyDown(sf::Keyboard::Key::Space)) {
        movement = movement + up;
    }
    if (input.isKeyDown(sf::Keyboard::Key::LShift) || input.isKeyDown(sf::Keyboard::Key::RShift)) {
        movement = movement - up;
    }

    if (movement.x != 0.f || movement.y != 0.f || movement.z != 0.f) {
        camera.setPosition(camera.position() + normalize(movement) * speed);
    }
}

} // namespace plunger
