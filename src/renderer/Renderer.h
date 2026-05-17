#pragma once

#include "renderer/Camera.h"
#include "renderer/Lighting.h"
#include "renderer/Mesh.h"
#include "renderer/PartRenderer.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "scene/Scene.h"

#include <cstdint>
#include <filesystem>

#include <SFML/OpenGL.hpp>
#include <SFML/Window/Window.hpp>

namespace plunger {

class Renderer {
public:
    void initialize(const std::filesystem::path& assetRoot);
    void reloadResources(const std::filesystem::path& assetRoot);
    void resize(sf::Vector2u size);
    void update(float deltaTime, float timeSeconds);
    void render(float timeSeconds);

    Camera& camera()
    {
        return m_camera;
    }

    const Camera& camera() const
    {
        return m_camera;
    }

    const Scene& scene() const
    {
        return m_scene;
    }
    // Non-const access to scene for runtime modification of parts/entities
    Scene& scene()
    {
        return m_scene;
    }

private:
    void initializeShadowResources();
    Mat4 buildLightSpaceMatrix() const;
    void renderShadowPass(const Mat4& lightSpace, float timeSeconds);

    std::filesystem::path m_assetRoot;
    Shader m_shader;
    Shader m_shadowShader;
    Shader m_partShadowShader;
    Mesh m_floorMesh;
    PartRenderer m_partRenderer;
    Texture m_texture;
    Camera m_camera;
    Scene m_scene;
    LightingEnvironment m_lighting;
    GLuint m_shadowFramebuffer = 0;
    GLuint m_shadowDepthTexture = 0;
    std::uint32_t m_shadowMapSize = 2048u;
    sf::Vector2u m_viewportSize {1u, 1u};
};

} // namespace plunger
