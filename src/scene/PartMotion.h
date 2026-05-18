#pragma once

#include "renderer/Math.h"
#include "scene/Components.h"

#include <cmath>
#include <cstddef>

namespace plunger {

inline Vec3 samplePartPosition(const Part& part, float timeSeconds, std::size_t partIndex)
{
    const float bob = part.bobAmplitude * std::sin(timeSeconds * 1.2f + static_cast<float>(partIndex) * 0.35f);

    Vec3 finalPosition = part.position;
    finalPosition.y += bob;

    if (part.circleRadius > 0.f) {
        const float circlePhase = timeSeconds * part.circleSpeed;
        if (part.circleAxisY > 0.5f) {
            finalPosition.x += part.circleRadius * std::cos(circlePhase);
            finalPosition.z += part.circleRadius * std::sin(circlePhase);
        } else {
            finalPosition.y += part.circleRadius * std::sin(circlePhase);
            finalPosition.z += part.circleRadius * std::cos(circlePhase);
        }
    }

    if (part.swayAmplitude > 0.f) {
        finalPosition.x += part.swayAmplitude * std::sin(timeSeconds * part.swaySpeed + static_cast<float>(partIndex) * 0.5f);
    }

    return finalPosition;
}

inline Mat4 composePartModelMatrix(const Part& part, float timeSeconds, std::size_t partIndex)
{
    const Vec3 finalPosition = samplePartPosition(part, timeSeconds, partIndex);

    float rotX = part.rotation.x;
    float rotY = part.rotation.y;
    float rotZ = part.rotation.z;

    if (part.spinSpeed != 0.f) {
        rotY += timeSeconds * part.spinSpeed;
    }

    const Mat4 translation = translate(finalPosition);
    const Mat4 rotationX = rotateX(rotX);
    const Mat4 rotationY = rotateY(rotY);
    const Mat4 rotationZ = rotateZ(rotZ);
    return multiply(translation, multiply(rotationY, multiply(rotationX, multiply(rotationZ, scale(part.size)))));
}

inline Vec3 transformPoint(const Mat4& matrix, const Vec3& point)
{
    const float x = matrix.values[0] * point.x + matrix.values[4] * point.y + matrix.values[8] * point.z + matrix.values[12];
    const float y = matrix.values[1] * point.x + matrix.values[5] * point.y + matrix.values[9] * point.z + matrix.values[13];
    const float z = matrix.values[2] * point.x + matrix.values[6] * point.y + matrix.values[10] * point.z + matrix.values[14];
    return {x, y, z};
}

inline void expandAabbWithPoint(Vec3& minCorner, Vec3& maxCorner, const Vec3& point)
{
    minCorner.x = std::min(minCorner.x, point.x);
    minCorner.y = std::min(minCorner.y, point.y);
    minCorner.z = std::min(minCorner.z, point.z);
    maxCorner.x = std::max(maxCorner.x, point.x);
    maxCorner.y = std::max(maxCorner.y, point.y);
    maxCorner.z = std::max(maxCorner.z, point.z);
}

inline void computeWorldAabb(const Mat4& modelMatrix, Vec3& minCorner, Vec3& maxCorner)
{
    minCorner = {1e9f, 1e9f, 1e9f};
    maxCorner = {-1e9f, -1e9f, -1e9f};

    for (int ix = 0; ix < 2; ++ix) {
        for (int iy = 0; iy < 2; ++iy) {
            for (int iz = 0; iz < 2; ++iz) {
                const Vec3 corner {
                    ix ? 0.5f : -0.5f,
                    iy ? 0.5f : -0.5f,
                    iz ? 0.5f : -0.5f,
                };
                expandAabbWithPoint(minCorner, maxCorner, transformPoint(modelMatrix, corner));
            }
        }
    }
}

} // namespace plunger
