#pragma once

#include "input/Input.h"
#include "renderer/Camera.h"

namespace plunger {

class FlyCameraController {
public:
    void update(Camera& camera, const Input& input, float deltaTime);

    float moveSpeed = 8.f;
    float mouseSensitivity = 0.0022f;
};

} // namespace plunger
