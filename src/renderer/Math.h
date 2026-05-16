#pragma once

#include <cmath>

namespace plunger {

struct Vec2 {
    float x = 0.f;
    float y = 0.f;
};

struct Vec3 {
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
};

struct Mat4 {
    float values[16]{};

    const float* data() const
    {
        return values;
    }

    static Mat4 identity()
    {
        Mat4 matrix;
        matrix.values[0] = 1.f;
        matrix.values[5] = 1.f;
        matrix.values[10] = 1.f;
        matrix.values[15] = 1.f;
        return matrix;
    }
};

inline Vec3 operator+(const Vec3& left, const Vec3& right)
{
    return {left.x + right.x, left.y + right.y, left.z + right.z};
}

inline Vec3 operator-(const Vec3& left, const Vec3& right)
{
    return {left.x - right.x, left.y - right.y, left.z - right.z};
}

inline Vec3 operator*(const Vec3& value, float scalar)
{
    return {value.x * scalar, value.y * scalar, value.z * scalar};
}

inline float dot(const Vec3& left, const Vec3& right)
{
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

inline Vec3 cross(const Vec3& left, const Vec3& right)
{
    return {
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x,
    };
}

inline float length(const Vec3& value)
{
    return std::sqrt(dot(value, value));
}

inline Vec3 normalize(const Vec3& value)
{
    const float len = length(value);
    if (len == 0.f) {
        return {0.f, 0.f, 0.f};
    }

    return {value.x / len, value.y / len, value.z / len};
}

inline Mat4 multiply(const Mat4& left, const Mat4& right)
{
    Mat4 result;
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.f;
            for (int index = 0; index < 4; ++index) {
                sum += left.values[index * 4 + row] * right.values[column * 4 + index];
            }
            result.values[column * 4 + row] = sum;
        }
    }
    return result;
}

inline Mat4 translate(const Vec3& offset)
{
    Mat4 matrix = Mat4::identity();
    matrix.values[12] = offset.x;
    matrix.values[13] = offset.y;
    matrix.values[14] = offset.z;
    return matrix;
}

inline Mat4 scale(const Vec3& factor)
{
    Mat4 matrix = Mat4::identity();
    matrix.values[0] = factor.x;
    matrix.values[5] = factor.y;
    matrix.values[10] = factor.z;
    return matrix;
}

inline Mat4 rotateX(float radians)
{
    Mat4 matrix = Mat4::identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    matrix.values[5] = c;
    matrix.values[6] = s;
    matrix.values[9] = -s;
    matrix.values[10] = c;
    return matrix;
}

inline Mat4 rotateY(float radians)
{
    Mat4 matrix = Mat4::identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    matrix.values[0] = c;
    matrix.values[2] = -s;
    matrix.values[8] = s;
    matrix.values[10] = c;
    return matrix;
}

inline Mat4 rotateZ(float radians)
{
    Mat4 matrix = Mat4::identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    matrix.values[0] = c;
    matrix.values[1] = s;
    matrix.values[4] = -s;
    matrix.values[5] = c;
    return matrix;
}

inline Mat4 perspective(float fieldOfViewRadians, float aspectRatio, float nearPlane, float farPlane)
{
    const float tangent = std::tan(fieldOfViewRadians * 0.5f);

    Mat4 matrix{};
    matrix.values[0] = 1.f / (aspectRatio * tangent);
    matrix.values[5] = 1.f / tangent;
    matrix.values[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    matrix.values[11] = -1.f;
    matrix.values[14] = -(2.f * farPlane * nearPlane) / (farPlane - nearPlane);
    return matrix;
}

inline Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up)
{
    const Vec3 forward = normalize(center - eye);
    const Vec3 side = normalize(cross(forward, up));
    const Vec3 cameraUp = cross(side, forward);

    Mat4 matrix = Mat4::identity();
    matrix.values[0] = side.x;
    matrix.values[4] = side.y;
    matrix.values[8] = side.z;

    matrix.values[1] = cameraUp.x;
    matrix.values[5] = cameraUp.y;
    matrix.values[9] = cameraUp.z;

    matrix.values[2] = -forward.x;
    matrix.values[6] = -forward.y;
    matrix.values[10] = -forward.z;

    matrix.values[12] = -dot(side, eye);
    matrix.values[13] = -dot(cameraUp, eye);
    matrix.values[14] = dot(forward, eye);
    return matrix;
}

} // namespace plunger