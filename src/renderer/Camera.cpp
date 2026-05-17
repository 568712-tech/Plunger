#include "renderer/Camera.h"

#include <algorithm>

namespace plunger {

void Camera::setPosition(const Vec3& position)
{
    m_position = position;
}

const Vec3& Camera::position() const
{
    return m_position;
}


void Camera::setYaw(float yawRadians)
{
    m_yaw = yawRadians;
}

void Camera::setPitch(float pitchRadians)
{
    const float limit = 1.55334306f;
    m_pitch = std::clamp(pitchRadians, -limit, limit);
}

void Camera::setAspectRatio(float aspectRatio)
{
    m_aspectRatio = aspectRatio;
}

void Camera::setFieldOfView(float fieldOfViewRadians)
{
    m_fieldOfView = fieldOfViewRadians;
}

Mat4 Camera::viewMatrix() const
{
    return lookAt(m_position, m_position + forward(), {0.f, 1.f, 0.f});
}

Mat4 Camera::projectionMatrix() const
{
    return perspective(m_fieldOfView, m_aspectRatio, 0.1f, 100.f);
}

void Camera::addYaw(float deltaRadians)
{
    m_yaw += deltaRadians;
    // Wrap yaw to [-PI, PI] to prevent floating point precision issues
    const float pi = 3.14159265f;
    while (m_yaw > pi) {
        m_yaw -= 2.0f * pi;
    }
    while (m_yaw < -pi) {
        m_yaw += 2.0f * pi;
    }
}

void Camera::addPitch(float deltaRadians)
{
    setPitch(m_pitch + deltaRadians);
}

Vec3 Camera::forward() const
{
    return {
        std::cos(m_pitch) * std::cos(m_yaw),
        std::sin(m_pitch),
        std::cos(m_pitch) * std::sin(m_yaw),
    };
}

} // namespace plunger