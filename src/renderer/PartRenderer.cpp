#include "renderer/PartRenderer.h"

#include "renderer/OpenGLFunctions.h"
#include "scene/PartMapLoader.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <vector>

namespace plunger {
namespace {

constexpr std::size_t shapeCount = static_cast<std::size_t>(PartShape::Count);
constexpr float pi = 3.14159265f;
constexpr float tau = 6.28318531f;
constexpr int radialSegments = 24;
constexpr int sphereLatitudeSegments = 16;
constexpr int torusMajorSegments = 24;
constexpr int torusMinorSegments = 16;
constexpr int capsuleHemisphereSegments = 8;

std::size_t shapeIndex(PartShape shape)
{
    return static_cast<std::size_t>(shape);
}

Mat4 composeModelMatrix(const Part& part, float timeSeconds, std::size_t index)
{
    // Calculate bobbing motion
    const float bob = part.bobAmplitude * std::sin(timeSeconds * 1.2f + static_cast<float>(index) * 0.35f);
    
    // Calculate position with all motion types
    Vec3 finalPosition = part.position;
    finalPosition.y += bob;
    
    // Circular motion
    if (part.circleRadius > 0.f) {
        const float circlePhase = timeSeconds * part.circleSpeed;
        if (part.circleAxisY > 0.5f) {
            // Rotate around Y axis (horizontal circle)
            finalPosition.x += part.circleRadius * std::cos(circlePhase);
            finalPosition.z += part.circleRadius * std::sin(circlePhase);
        } else {
            // Rotate around X axis (vertical circle in XY plane)
            finalPosition.y += part.circleRadius * std::sin(circlePhase);
            finalPosition.z += part.circleRadius * std::cos(circlePhase);
        }
    }
    
    // Side-to-side sway motion
    if (part.swayAmplitude > 0.f) {
        finalPosition.x += part.swayAmplitude * std::sin(timeSeconds * part.swaySpeed + static_cast<float>(index) * 0.5f);
    }
    
    const Mat4 translation = translate(finalPosition);
    
    // Calculate rotation with spin
    float rotX = part.rotation.x;
    float rotY = part.rotation.y;
    float rotZ = part.rotation.z;
    
    if (part.spinSpeed != 0.f) {
        // Add spin to Y rotation
        rotY += timeSeconds * part.spinSpeed;
    }
    
    const Mat4 rotationX = rotateX(rotX);
    const Mat4 rotationY = rotateY(rotY);
    const Mat4 rotationZ = rotateZ(rotZ);
    return multiply(translation, multiply(rotationY, multiply(rotationX, multiply(rotationZ, scale(part.size)))));
}

void appendTriangle(std::vector<PartRenderer::Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const Vec3& a,
    const Vec3& b,
    const Vec3& c)
{
    const Vec3 normal = normalize(cross(b - a, c - a));
    const unsigned int start = static_cast<unsigned int>(vertices.size());
    vertices.push_back({a, normal, {0.f, 0.f}});
    vertices.push_back({b, normal, {1.f, 0.f}});
    vertices.push_back({c, normal, {0.5f, 1.f}});
    indices.push_back(start);
    indices.push_back(start + 1u);
    indices.push_back(start + 2u);
}

void appendQuad(std::vector<PartRenderer::Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const Vec3& a,
    const Vec3& b,
    const Vec3& c,
    const Vec3& d)
{
    const Vec3 normal = normalize(cross(b - a, c - a));
    const unsigned int start = static_cast<unsigned int>(vertices.size());
    vertices.push_back({a, normal, {0.f, 0.f}});
    vertices.push_back({b, normal, {1.f, 0.f}});
    vertices.push_back({c, normal, {1.f, 1.f}});
    vertices.push_back({d, normal, {0.f, 1.f}});
    indices.push_back(start);
    indices.push_back(start + 1u);
    indices.push_back(start + 2u);
    indices.push_back(start + 2u);
    indices.push_back(start + 3u);
    indices.push_back(start);
}

void appendFlatPolygon(std::vector<PartRenderer::Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const std::vector<Vec3>& points,
    const Vec3& normal)
{
    if (points.size() < 3u) {
        return;
    }

    const unsigned int start = static_cast<unsigned int>(vertices.size());
    for (const Vec3& point : points) {
        vertices.push_back({point, normal, {point.x + 0.5f, point.z + 0.5f}});
    }

    for (std::size_t index = 1; index + 1 < points.size(); ++index) {
        indices.push_back(start);
        indices.push_back(start + static_cast<unsigned int>(index));
        indices.push_back(start + static_cast<unsigned int>(index + 1));
    }
}

void appendRevolvedSideQuad(std::vector<PartRenderer::Vertex>& vertices,
    std::vector<unsigned int>& indices,
    const Vec3& lower0,
    const Vec3& lower1,
    const Vec3& upper1,
    const Vec3& upper0,
    float u0,
    float u1,
    float v0,
    float v1)
{
    const Vec3 normal0 = normalize({lower0.x, 0.f, lower0.z});
    const Vec3 normal1 = normalize({lower1.x, 0.f, lower1.z});
    const unsigned int start = static_cast<unsigned int>(vertices.size());
    vertices.push_back({lower0, normal0, {u0, v0}});
    vertices.push_back({lower1, normal1, {u1, v0}});
    vertices.push_back({upper1, normal1, {u1, v1}});
    vertices.push_back({upper0, normal0, {u0, v1}});
    indices.push_back(start);
    indices.push_back(start + 1u);
    indices.push_back(start + 2u);
    indices.push_back(start + 2u);
    indices.push_back(start + 3u);
    indices.push_back(start);
}

PartRenderer::Geometry buildBoxGeometry()
{
    PartRenderer::Geometry geometry;
    geometry.vertices = {
        {{-0.5f, -0.5f,  0.5f}, {0.f, 0.f, 1.f}, {0.f, 0.f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.f, 0.f, 1.f}, {1.f, 0.f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f}, {1.f, 1.f}},
        {{-0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f}, {0.f, 1.f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.f, 0.f,-1.f}, {0.f, 0.f}},
        {{-0.5f, -0.5f, -0.5f}, {0.f, 0.f,-1.f}, {1.f, 0.f}},
        {{-0.5f,  0.5f, -0.5f}, {0.f, 0.f,-1.f}, {1.f, 1.f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.f, 0.f,-1.f}, {0.f, 1.f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 0.f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.f, 0.f, 0.f}, {1.f, 0.f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.f, 0.f, 0.f}, {1.f, 1.f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 1.f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.f, 0.f, 0.f}, {0.f, 0.f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 0.f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 1.f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.f, 0.f, 0.f}, {0.f, 1.f}},
        {{-0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f}, {1.f, 0.f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f}, {1.f, 1.f}},
        {{-0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f}, {0.f, 1.f}},
        {{-0.5f, -0.5f, -0.5f}, {0.f,-1.f, 0.f}, {0.f, 0.f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.f,-1.f, 0.f}, {1.f, 0.f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.f,-1.f, 0.f}, {1.f, 1.f}},
        {{-0.5f, -0.5f,  0.5f}, {0.f,-1.f, 0.f}, {0.f, 1.f}},
    };

    geometry.indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };
    return geometry;
}

PartRenderer::Geometry buildCylinderGeometry()
{
    PartRenderer::Geometry geometry;
    std::vector<Vec3> topRing;
    std::vector<Vec3> bottomRing;
    topRing.reserve(radialSegments);
    bottomRing.reserve(radialSegments);

    for (int segment = 0; segment < radialSegments; ++segment) {
        const float angle = tau * static_cast<float>(segment) / static_cast<float>(radialSegments);
        const float x = 0.5f * std::cos(angle);
        const float z = 0.5f * std::sin(angle);
        topRing.push_back({x, 0.5f, z});
        bottomRing.push_back({x, -0.5f, z});
    }

    for (int segment = 0; segment < radialSegments; ++segment) {
        const int next = (segment + 1) % radialSegments;
        appendRevolvedSideQuad(geometry.vertices,
            geometry.indices,
            bottomRing[segment],
            bottomRing[next],
            topRing[next],
            topRing[segment],
            static_cast<float>(segment) / static_cast<float>(radialSegments),
            static_cast<float>(segment + 1) / static_cast<float>(radialSegments),
            0.f,
            1.f);
    }

    appendFlatPolygon(geometry.vertices, geometry.indices, topRing, {0.f, 1.f, 0.f});
    appendFlatPolygon(geometry.vertices, geometry.indices, bottomRing, {0.f, -1.f, 0.f});
    return geometry;
}

PartRenderer::Geometry buildPyramidGeometry()
{
    PartRenderer::Geometry geometry;
    const Vec3 apex {0.f, 0.5f, 0.f};
    const Vec3 frontLeft {-0.5f, -0.5f, 0.5f};
    const Vec3 frontRight {0.5f, -0.5f, 0.5f};
    const Vec3 backRight {0.5f, -0.5f, -0.5f};
    const Vec3 backLeft {-0.5f, -0.5f, -0.5f};

    appendTriangle(geometry.vertices, geometry.indices, frontLeft, frontRight, apex);
    appendTriangle(geometry.vertices, geometry.indices, frontRight, backRight, apex);
    appendTriangle(geometry.vertices, geometry.indices, backRight, backLeft, apex);
    appendTriangle(geometry.vertices, geometry.indices, backLeft, frontLeft, apex);
    appendQuad(geometry.vertices, geometry.indices, backLeft, backRight, frontRight, frontLeft);
    return geometry;
}

PartRenderer::Geometry buildWedgeGeometry()
{
    PartRenderer::Geometry geometry;
    const Vec3 frontBottomLeft {-0.5f, -0.5f, 0.5f};
    const Vec3 frontBottomRight {0.5f, -0.5f, 0.5f};
    const Vec3 backBottomLeft {-0.5f, -0.5f, -0.5f};
    const Vec3 backBottomRight {0.5f, -0.5f, -0.5f};
    const Vec3 backTopLeft {-0.5f, 0.5f, -0.5f};
    const Vec3 backTopRight {0.5f, 0.5f, -0.5f};

    appendQuad(geometry.vertices, geometry.indices, backBottomLeft, backBottomRight, frontBottomRight, frontBottomLeft);
    appendQuad(geometry.vertices, geometry.indices, backBottomLeft, backTopLeft, backTopRight, backBottomRight);
    appendQuad(geometry.vertices, geometry.indices, backTopLeft, frontBottomLeft, frontBottomRight, backTopRight);
    appendTriangle(geometry.vertices, geometry.indices, backBottomLeft, frontBottomLeft, backTopLeft);
    appendTriangle(geometry.vertices, geometry.indices, frontBottomRight, backBottomRight, backTopRight);
    return geometry;
}

PartRenderer::Geometry buildConeGeometry()
{
    PartRenderer::Geometry geometry;
    const Vec3 apex {0.f, 0.5f, 0.f};
    std::vector<Vec3> baseRing;
    baseRing.reserve(radialSegments);

    for (int segment = 0; segment < radialSegments; ++segment) {
        const float angle = tau * static_cast<float>(segment) / static_cast<float>(radialSegments);
        baseRing.push_back({0.5f * std::cos(angle), -0.5f, 0.5f * std::sin(angle)});
    }

    for (int segment = 0; segment < radialSegments; ++segment) {
        const int next = (segment + 1) % radialSegments;
        appendTriangle(geometry.vertices, geometry.indices, baseRing[segment], baseRing[next], apex);
    }

    appendFlatPolygon(geometry.vertices, geometry.indices, baseRing, {0.f, -1.f, 0.f});
    return geometry;
}

PartRenderer::Geometry buildUvSphereGeometry(float minPhi, float maxPhi)
{
    PartRenderer::Geometry geometry;
    const int latSegments = maxPhi < pi ? sphereLatitudeSegments / 2 : sphereLatitudeSegments;

    for (int latitude = 0; latitude <= latSegments; ++latitude) {
        const float v = static_cast<float>(latitude) / static_cast<float>(latSegments);
        const float phi = minPhi + (maxPhi - minPhi) * v;
        const float ringRadius = 0.5f * std::sin(phi);
        const float y = 0.5f * std::cos(phi);

        for (int longitude = 0; longitude <= radialSegments; ++longitude) {
            const float u = static_cast<float>(longitude) / static_cast<float>(radialSegments);
            const float theta = tau * u;
            const Vec3 position {ringRadius * std::cos(theta), y, ringRadius * std::sin(theta)};
            geometry.vertices.push_back({position, normalize(position), {u, v}});
        }
    }

    const int stride = radialSegments + 1;
    for (int latitude = 0; latitude < latSegments; ++latitude) {
        for (int longitude = 0; longitude < radialSegments; ++longitude) {
            const unsigned int current = static_cast<unsigned int>(latitude * stride + longitude);
            const unsigned int next = current + 1u;
            const unsigned int below = current + static_cast<unsigned int>(stride);
            const unsigned int belowNext = below + 1u;
            geometry.indices.push_back(current);
            geometry.indices.push_back(below);
            geometry.indices.push_back(next);
            geometry.indices.push_back(next);
            geometry.indices.push_back(below);
            geometry.indices.push_back(belowNext);
        }
    }

    if (maxPhi < pi) {
        std::vector<Vec3> baseRing;
        baseRing.reserve(radialSegments);
        for (int longitude = 0; longitude < radialSegments; ++longitude) {
            const float u = static_cast<float>(longitude) / static_cast<float>(radialSegments);
            const float theta = tau * u;
            baseRing.push_back({0.5f * std::cos(theta), 0.f, 0.5f * std::sin(theta)});
        }
        appendFlatPolygon(geometry.vertices, geometry.indices, baseRing, {0.f, -1.f, 0.f});
    }

    return geometry;
}

PartRenderer::Geometry buildSphereGeometry()
{
    return buildUvSphereGeometry(0.f, pi);
}

PartRenderer::Geometry buildHemisphereGeometry()
{
    return buildUvSphereGeometry(0.f, pi * 0.5f);
}

PartRenderer::Geometry buildTorusGeometry()
{
    PartRenderer::Geometry geometry;
    constexpr float majorRadius = 0.35f;
    constexpr float minorRadius = 0.15f;

    for (int major = 0; major <= torusMajorSegments; ++major) {
        const float u = static_cast<float>(major) / static_cast<float>(torusMajorSegments);
        const float theta = tau * u;
        const float cosTheta = std::cos(theta);
        const float sinTheta = std::sin(theta);

        for (int minor = 0; minor <= torusMinorSegments; ++minor) {
            const float v = static_cast<float>(minor) / static_cast<float>(torusMinorSegments);
            const float phi = tau * v;
            const float cosPhi = std::cos(phi);
            const float sinPhi = std::sin(phi);
            const float radius = majorRadius + minorRadius * cosPhi;
            const Vec3 position {radius * cosTheta, minorRadius * sinPhi, radius * sinTheta};
            const Vec3 normal {cosTheta * cosPhi, sinPhi, sinTheta * cosPhi};
            geometry.vertices.push_back({position, normalize(normal), {u, v}});
        }
    }

    const int stride = torusMinorSegments + 1;
    for (int major = 0; major < torusMajorSegments; ++major) {
        for (int minor = 0; minor < torusMinorSegments; ++minor) {
            const unsigned int current = static_cast<unsigned int>(major * stride + minor);
            const unsigned int next = current + 1u;
            const unsigned int below = current + static_cast<unsigned int>(stride);
            const unsigned int belowNext = below + 1u;
            geometry.indices.push_back(current);
            geometry.indices.push_back(below);
            geometry.indices.push_back(next);
            geometry.indices.push_back(next);
            geometry.indices.push_back(below);
            geometry.indices.push_back(belowNext);
        }
    }

    return geometry;
}

PartRenderer::Geometry buildPrismGeometry(int sides)
{
    PartRenderer::Geometry geometry;
    std::vector<Vec3> topRing;
    std::vector<Vec3> bottomRing;
    topRing.reserve(sides);
    bottomRing.reserve(sides);

    const float angleOffset = sides == 3 ? pi * 0.5f : 0.f;
    for (int segment = 0; segment < sides; ++segment) {
        const float angle = angleOffset + tau * static_cast<float>(segment) / static_cast<float>(sides);
        const float x = 0.5f * std::cos(angle);
        const float z = 0.5f * std::sin(angle);
        topRing.push_back({x, 0.5f, z});
        bottomRing.push_back({x, -0.5f, z});
    }

    for (int segment = 0; segment < sides; ++segment) {
        const int next = (segment + 1) % sides;
        appendQuad(geometry.vertices,
            geometry.indices,
            bottomRing[segment],
            bottomRing[next],
            topRing[next],
            topRing[segment]);
    }

    appendFlatPolygon(geometry.vertices, geometry.indices, topRing, {0.f, 1.f, 0.f});
    appendFlatPolygon(geometry.vertices, geometry.indices, bottomRing, {0.f, -1.f, 0.f});
    return geometry;
}

PartRenderer::Geometry buildOctahedronGeometry()
{
    PartRenderer::Geometry geometry;
    const Vec3 top {0.f, 0.5f, 0.f};
    const Vec3 bottom {0.f, -0.5f, 0.f};
    const Vec3 front {0.f, 0.f, 0.5f};
    const Vec3 right {0.5f, 0.f, 0.f};
    const Vec3 back {0.f, 0.f, -0.5f};
    const Vec3 left {-0.5f, 0.f, 0.f};

    appendTriangle(geometry.vertices, geometry.indices, front, right, top);
    appendTriangle(geometry.vertices, geometry.indices, right, back, top);
    appendTriangle(geometry.vertices, geometry.indices, back, left, top);
    appendTriangle(geometry.vertices, geometry.indices, left, front, top);
    appendTriangle(geometry.vertices, geometry.indices, right, front, bottom);
    appendTriangle(geometry.vertices, geometry.indices, back, right, bottom);
    appendTriangle(geometry.vertices, geometry.indices, left, back, bottom);
    appendTriangle(geometry.vertices, geometry.indices, front, left, bottom);
    return geometry;
}

PartRenderer::Geometry buildCapsuleGeometry()
{
    PartRenderer::Geometry geometry;
    constexpr float radius = 0.25f;
    constexpr float cylinderHalfHeight = 0.25f;

    for (int segment = 0; segment < radialSegments; ++segment) {
        const int next = (segment + 1) % radialSegments;
        const float angle0 = tau * static_cast<float>(segment) / static_cast<float>(radialSegments);
        const float angle1 = tau * static_cast<float>(next) / static_cast<float>(radialSegments);
        const Vec3 lower0 {radius * std::cos(angle0), -cylinderHalfHeight, radius * std::sin(angle0)};
        const Vec3 lower1 {radius * std::cos(angle1), -cylinderHalfHeight, radius * std::sin(angle1)};
        const Vec3 upper0 {radius * std::cos(angle0), cylinderHalfHeight, radius * std::sin(angle0)};
        const Vec3 upper1 {radius * std::cos(angle1), cylinderHalfHeight, radius * std::sin(angle1)};
        appendRevolvedSideQuad(geometry.vertices,
            geometry.indices,
            lower0,
            lower1,
            upper1,
            upper0,
            static_cast<float>(segment) / static_cast<float>(radialSegments),
            static_cast<float>(segment + 1) / static_cast<float>(radialSegments),
            0.f,
            1.f);
    }

    for (int latitude = 0; latitude < capsuleHemisphereSegments; ++latitude) {
        const float phi0 = (pi * 0.5f) * static_cast<float>(latitude) / static_cast<float>(capsuleHemisphereSegments);
        const float phi1 = (pi * 0.5f) * static_cast<float>(latitude + 1) / static_cast<float>(capsuleHemisphereSegments);
        const float y0 = std::sin(phi0) * radius;
        const float y1 = std::sin(phi1) * radius;
        const float ring0 = std::cos(phi0) * radius;
        const float ring1 = std::cos(phi1) * radius;

        for (int segment = 0; segment < radialSegments; ++segment) {
            const int next = (segment + 1) % radialSegments;
            const float angle0 = tau * static_cast<float>(segment) / static_cast<float>(radialSegments);
            const float angle1 = tau * static_cast<float>(next) / static_cast<float>(radialSegments);

            const Vec3 topLower0 {ring0 * std::cos(angle0), cylinderHalfHeight + y0, ring0 * std::sin(angle0)};
            const Vec3 topLower1 {ring0 * std::cos(angle1), cylinderHalfHeight + y0, ring0 * std::sin(angle1)};
            const Vec3 topUpper0 {ring1 * std::cos(angle0), cylinderHalfHeight + y1, ring1 * std::sin(angle0)};
            const Vec3 topUpper1 {ring1 * std::cos(angle1), cylinderHalfHeight + y1, ring1 * std::sin(angle1)};

            const Vec3 bottomUpper0 {ring0 * std::cos(angle0), -cylinderHalfHeight - y0, ring0 * std::sin(angle0)};
            const Vec3 bottomUpper1 {ring0 * std::cos(angle1), -cylinderHalfHeight - y0, ring0 * std::sin(angle1)};
            const Vec3 bottomLower0 {ring1 * std::cos(angle0), -cylinderHalfHeight - y1, ring1 * std::sin(angle0)};
            const Vec3 bottomLower1 {ring1 * std::cos(angle1), -cylinderHalfHeight - y1, ring1 * std::sin(angle1)};

            const unsigned int startTop = static_cast<unsigned int>(geometry.vertices.size());
            geometry.vertices.push_back({topLower0, normalize({topLower0.x, topLower0.y - cylinderHalfHeight, topLower0.z}), {0.f, 0.f}});
            geometry.vertices.push_back({topLower1, normalize({topLower1.x, topLower1.y - cylinderHalfHeight, topLower1.z}), {1.f, 0.f}});
            geometry.vertices.push_back({topUpper1, normalize({topUpper1.x, topUpper1.y - cylinderHalfHeight, topUpper1.z}), {1.f, 1.f}});
            geometry.vertices.push_back({topUpper0, normalize({topUpper0.x, topUpper0.y - cylinderHalfHeight, topUpper0.z}), {0.f, 1.f}});
            geometry.indices.push_back(startTop);
            geometry.indices.push_back(startTop + 1u);
            geometry.indices.push_back(startTop + 2u);
            geometry.indices.push_back(startTop + 2u);
            geometry.indices.push_back(startTop + 3u);
            geometry.indices.push_back(startTop);

            const unsigned int startBottom = static_cast<unsigned int>(geometry.vertices.size());
            geometry.vertices.push_back({bottomUpper0, normalize({bottomUpper0.x, bottomUpper0.y + cylinderHalfHeight, bottomUpper0.z}), {0.f, 0.f}});
            geometry.vertices.push_back({bottomUpper1, normalize({bottomUpper1.x, bottomUpper1.y + cylinderHalfHeight, bottomUpper1.z}), {1.f, 0.f}});
            geometry.vertices.push_back({bottomLower1, normalize({bottomLower1.x, bottomLower1.y + cylinderHalfHeight, bottomLower1.z}), {1.f, 1.f}});
            geometry.vertices.push_back({bottomLower0, normalize({bottomLower0.x, bottomLower0.y + cylinderHalfHeight, bottomLower0.z}), {0.f, 1.f}});
            geometry.indices.push_back(startBottom);
            geometry.indices.push_back(startBottom + 2u);
            geometry.indices.push_back(startBottom + 1u);
            geometry.indices.push_back(startBottom);
            geometry.indices.push_back(startBottom + 3u);
            geometry.indices.push_back(startBottom + 2u);
        }
    }

    return geometry;
}

PartRenderer::Geometry buildGeometryForShape(PartShape shape)
{
    switch (shape) {
    case PartShape::Box:
        return buildBoxGeometry();
    case PartShape::Cylinder:
        return buildCylinderGeometry();
    case PartShape::Pyramid:
        return buildPyramidGeometry();
    case PartShape::Wedge:
        return buildWedgeGeometry();
    case PartShape::Cone:
        return buildConeGeometry();
    case PartShape::Sphere:
        return buildSphereGeometry();
    case PartShape::Hemisphere:
        return buildHemisphereGeometry();
    case PartShape::Torus:
        return buildTorusGeometry();
    case PartShape::TriPrism:
        return buildPrismGeometry(3);
    case PartShape::HexPrism:
        return buildPrismGeometry(6);
    case PartShape::Octahedron:
        return buildOctahedronGeometry();
    case PartShape::Capsule:
        return buildCapsuleGeometry();
    case PartShape::Count:
        return buildBoxGeometry();
    }

    return buildBoxGeometry();
}

void drawGeometryInstanced(const PartRenderer::Geometry& geometry)
{
    if (geometry.instances.empty()) {
        return;
    }

    gl::bindVertexArray(geometry.vertexArray);
    gl::drawElementsInstanced(GL_TRIANGLES,
        static_cast<GLsizei>(geometry.indices.size()),
        GL_UNSIGNED_INT,
        nullptr,
        static_cast<GLsizei>(geometry.instances.size()));
    gl::bindVertexArray(0);
}

} // namespace

void PartRenderer::initialize(Scene& scene, const std::filesystem::path& assetRoot, const std::filesystem::path& mapPath)
{
    m_scene = &scene;

    const std::vector<Part> parts = PartMapLoader::loadJson(mapPath);
    for (const Part& part : parts) {
        const EntityId entityId = m_scene->createEntity();
        m_scene->addPart(entityId, part);
    }

    uploadGeometry();
    m_shader.loadFromFiles(assetRoot / "shaders" / "part.vert", assetRoot / "shaders" / "part.frag");
}

void PartRenderer::renderShadowPass(Shader& shadowShader, const Mat4& lightSpace, float timeSeconds) const
{
    uploadInstances(timeSeconds);

    glDisable(GL_CULL_FACE);
    shadowShader.bind();
    shadowShader.setMat4("uLightSpace", lightSpace);

    for (const Geometry& geometry : m_geometries) {
        drawGeometryInstanced(geometry);
    }

    shadowShader.unbind();
    glEnable(GL_CULL_FACE);
}

void PartRenderer::render(const Mat4& view,
    const Mat4& projection,
    const Mat4& lightSpace,
    const Vec3& cameraPosition,
    const LightingEnvironment& lighting,
    GLuint shadowMapTexture,
    float timeSeconds) const
{
    uploadInstances(timeSeconds);

    glDisable(GL_CULL_FACE);
    m_shader.bind();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    m_shader.setMat4("uLightSpace", lightSpace);
    m_shader.setVec3("uCameraPosition", cameraPosition);
    lighting.apply(m_shader);
    m_shader.setInt("uShadowMap", 1);
    gl::activeTexture(GL_TEXTURE0 + 1);
    gl::bindTexture(GL_TEXTURE_2D, shadowMapTexture);

    for (const Geometry& geometry : m_geometries) {
        drawGeometryInstanced(geometry);
    }

    m_shader.unbind();
    glEnable(GL_CULL_FACE);
}

void PartRenderer::reloadResources(const std::filesystem::path& assetRoot)
{
    m_shader.loadFromFiles(assetRoot / "shaders" / "part.vert", assetRoot / "shaders" / "part.frag");
    releaseGeometry();
    uploadGeometry();
}

void PartRenderer::releaseGeometry()
{
    for (Geometry& geometry : m_geometries) {
        releaseGeometryBuffers(geometry);
    }
}

void PartRenderer::uploadGeometry()
{
    for (std::size_t index = 0; index < shapeCount; ++index) {
        m_geometries[index] = buildGeometryForShape(static_cast<PartShape>(index));
        uploadGeometryBuffers(m_geometries[index]);
    }
}

void PartRenderer::uploadGeometryBuffers(Geometry& geometry)
{
    gl::genVertexArrays(1, &geometry.vertexArray);
    gl::genBuffers(1, &geometry.vertexBuffer);
    gl::genBuffers(1, &geometry.indexBuffer);
    gl::genBuffers(1, &geometry.instanceBuffer);

    gl::bindVertexArray(geometry.vertexArray);

    gl::bindBuffer(GL_ARRAY_BUFFER, geometry.vertexBuffer);
    gl::bufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(geometry.vertices.size() * sizeof(Vertex)),
        geometry.vertices.data(),
        GL_STATIC_DRAW);

    gl::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.indexBuffer);
    gl::bufferData(GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(geometry.indices.size() * sizeof(unsigned int)),
        geometry.indices.data(),
        GL_STATIC_DRAW);

    gl::enableVertexAttribArray(0);
    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    gl::enableVertexAttribArray(1);
    gl::vertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    gl::enableVertexAttribArray(2);
    gl::vertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

    gl::bindBuffer(GL_ARRAY_BUFFER, geometry.instanceBuffer);
    gl::bufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    const std::size_t matrixStride = sizeof(InstanceData);
    for (GLuint column = 0; column < 4; ++column) {
        const GLuint location = 3u + column;
        gl::enableVertexAttribArray(location);
        gl::vertexAttribPointer(location,
            4,
            GL_FLOAT,
            GL_FALSE,
            static_cast<GLsizei>(matrixStride),
            reinterpret_cast<void*>(column * 4u * sizeof(float)));
        gl::vertexAttribDivisor(location, 1);
    }

    gl::enableVertexAttribArray(7);
    gl::vertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(matrixStride), reinterpret_cast<void*>(sizeof(Mat4)));
    gl::vertexAttribDivisor(7, 1);

    gl::enableVertexAttribArray(8);
    gl::vertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(matrixStride), reinterpret_cast<void*>(sizeof(Mat4) + sizeof(Vec3)));
    gl::vertexAttribDivisor(8, 1);

    gl::enableVertexAttribArray(9);
    gl::vertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(matrixStride), reinterpret_cast<void*>(sizeof(Mat4) + sizeof(Vec3) + sizeof(float)));
    gl::vertexAttribDivisor(9, 1);

    gl::bindVertexArray(0);
}

void PartRenderer::releaseGeometryBuffers(Geometry& geometry)
{
    if (geometry.instanceBuffer != 0) {
        gl::deleteBuffers(1, &geometry.instanceBuffer);
        geometry.instanceBuffer = 0;
    }
    if (geometry.indexBuffer != 0) {
        gl::deleteBuffers(1, &geometry.indexBuffer);
        geometry.indexBuffer = 0;
    }
    if (geometry.vertexBuffer != 0) {
        gl::deleteBuffers(1, &geometry.vertexBuffer);
        geometry.vertexBuffer = 0;
    }
    if (geometry.vertexArray != 0) {
        gl::deleteVertexArrays(1, &geometry.vertexArray);
        geometry.vertexArray = 0;
    }
}

void PartRenderer::uploadInstances(float timeSeconds) const
{
    if (m_scene == nullptr) {
        return;
    }

    std::array<std::size_t, shapeCount> counts {};
    m_scene->forEachPart([&](const Entity&, const Part& part) {
        ++counts[shapeIndex(part.shape)];
    });

    for (std::size_t index = 0; index < shapeCount; ++index) {
        m_geometries[index].instances.resize(counts[index]);
    }

    std::array<std::size_t, shapeCount> writeIndices {};
    std::size_t partIndex = 0;
    m_scene->forEachPart([&](const Entity&, const Part& part) {
        const std::size_t geometryIndex = shapeIndex(part.shape);
        Geometry& geometry = m_geometries[geometryIndex];
        InstanceData& instance = geometry.instances[writeIndices[geometryIndex]++];
        instance.model = composeModelMatrix(part, timeSeconds, partIndex);
        instance.color = part.material.baseColor;
        instance.roughness = part.material.roughness;
        instance.metallic = part.material.metallic;
        ++partIndex;
    });

    for (Geometry& geometry : m_geometries) {
        gl::bindBuffer(GL_ARRAY_BUFFER, geometry.instanceBuffer);
        gl::bufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(geometry.instances.size() * sizeof(InstanceData)),
            geometry.instances.empty() ? nullptr : geometry.instances.data(),
            GL_DYNAMIC_DRAW);
    }
}

} // namespace plunger
