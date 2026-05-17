#pragma once

#include "renderer/Lighting.h"
#include "renderer/Math.h"
#include "renderer/Shader.h"
#include "scene/Components.h"
#include "scene/Scene.h"

#include <array>
#include <filesystem>
#include <vector>

#include <SFML/OpenGL.hpp>

namespace plunger {

class PartRenderer {
public:
    void initialize(Scene& scene, const std::filesystem::path& assetRoot, const std::filesystem::path& mapPath);
    void reloadResources(const std::filesystem::path& assetRoot);
    void renderShadowPass(Shader& shadowShader, const Mat4& lightSpace, float timeSeconds) const;
    void render(const Mat4& view,
        const Mat4& projection,
        const Mat4& lightSpace,
        const Vec3& cameraPosition,
        const LightingEnvironment& lighting,
        GLuint shadowMapTexture,
        float timeSeconds) const;

    struct Vertex {
        Vec3 position;
        Vec3 normal;
        Vec2 texCoord;
    };

    struct InstanceData {
        Mat4 model;
        Vec3 color;
        float roughness = 1.f;
        float metallic = 0.f;
        float padding[3] {0.f, 0.f, 0.f};
    };

    struct Geometry {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        mutable std::vector<InstanceData> instances;
        GLuint vertexArray = 0;
        GLuint vertexBuffer = 0;
        GLuint indexBuffer = 0;
        GLuint instanceBuffer = 0;
    };

private:
    static constexpr std::size_t shapeCount = static_cast<std::size_t>(PartShape::Count);

    void uploadGeometry();
    void uploadInstances(float timeSeconds) const;
    void releaseGeometry();
    void uploadGeometryBuffers(Geometry& geometry);
    void releaseGeometryBuffers(Geometry& geometry);

    Scene* m_scene = nullptr;
    Shader m_shader;
    mutable std::array<Geometry, shapeCount> m_geometries;
};

} // namespace plunger
