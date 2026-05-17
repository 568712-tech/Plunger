#pragma once

#include "renderer/Lighting.h"
#include "renderer/Math.h"
#include "renderer/Shader.h"
#include "scene/Components.h"
#include "scene/Scene.h"

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

private:
    struct InstanceData {
        Mat4 model;
        Vec3 color;
        float padding = 0.f;
    };

    void uploadGeometry();
    void uploadInstances(float timeSeconds) const;
    void releaseGeometry();

    Scene* m_scene = nullptr;
    mutable std::vector<InstanceData> m_instances;
    Shader m_shader;
    GLuint m_vertexArray = 0;
    GLuint m_vertexBuffer = 0;
    GLuint m_indexBuffer = 0;
    GLuint m_instanceBuffer = 0;
};

} // namespace plunger
