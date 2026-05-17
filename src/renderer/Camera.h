#pragma once

#include "renderer/Math.h"

namespace plunger {

class Camera {
public:
    void setPosition(const Vec3& position);
    const Vec3& position() const;
    void setYaw(float yawRadians);
    void setPitch(float pitchRadians);
    void setAspectRatio(float aspectRatio);
    void setFieldOfView(float fieldOfViewRadians);

    Mat4 viewMatrix() const;
    Mat4 projectionMatrix() const;
    Vec3 forward() const;
    void addYaw(float deltaRadians);
    void addPitch(float deltaRadians);

private:
    Vec3 m_position {0.f, 1.5f, 5.f};
    float m_yaw = -1.57079633f;
    float m_pitch = 0.f;
    float m_aspectRatio = 16.f / 9.f;
    float m_fieldOfView = 1.04719755f;
};

} // namespace plunger