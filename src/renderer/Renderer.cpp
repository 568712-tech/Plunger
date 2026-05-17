#include "renderer/Renderer.h"

#include "renderer/OpenGLFunctions.h"
#include <cmath>
#include <stdexcept>

namespace plunger {

namespace {
constexpr GLenum GL_NONE_ENUM = 0;
}

void Renderer::initializeShadowResources()
{
    gl::genFramebuffers(1, &m_shadowFramebuffer);
    gl::genTextures(1, &m_shadowDepthTexture);

    gl::bindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);
    gl::texImage2D(GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        static_cast<GLsizei>(m_shadowMapSize),
        static_cast<GLsizei>(m_shadowMapSize),
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr);
    gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float borderColor[4] = {1.f, 1.f, 1.f, 1.f};
    gl::texParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    gl::bindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
    gl::framebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowDepthTexture, 0);
    glDrawBuffer(GL_NONE_ENUM);
    glReadBuffer(GL_NONE_ENUM);

    if (gl::checkFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Failed to create shadow framebuffer");
    }

    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);
}

Mat4 Renderer::buildLightSpaceMatrix() const
{
    const Vec3 lightDirection = normalize(m_lighting.lightDirection);
    // Position light further back to cover more of the scene
    const Vec3 lightPosition = lightDirection * -35.f;
    // Focus on center of the obstacle course
    const Vec3 lookAtPoint = {0.f, 0.f, 15.f};
    const Mat4 lightView = lookAt(lightPosition, lookAtPoint, {0.f, 1.f, 0.f});
    // Increased bounds to cover the entire obstacle course from Z: -30 to Z: 62
    const Mat4 lightProjection = orthographic(-30.f, 30.f, -15.f, 25.f, 1.f, 80.f);
    return multiply(lightProjection, lightView);
}

void Renderer::renderShadowPass(const Mat4& lightSpace, float timeSeconds)
{
    glViewport(0, 0, static_cast<GLsizei>(m_shadowMapSize), static_cast<GLsizei>(m_shadowMapSize));
    gl::bindFramebuffer(GL_FRAMEBUFFER, m_shadowFramebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);

    glCullFace(GL_FRONT);

    m_shadowShader.bind();
    m_shadowShader.setMat4("uLightSpace", lightSpace);
    m_scene.forEachRenderable([&](const Entity&, const Transform& transform, const MeshRenderer& meshRenderer) {
        m_shadowShader.setMat4("uModel", transform.worldMatrix);
        meshRenderer.mesh->draw();
    });
    m_shadowShader.unbind();

    m_partRenderer.renderShadowPass(m_partShadowShader, lightSpace, timeSeconds);

    glCullFace(GL_BACK);
    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, static_cast<GLsizei>(m_viewportSize.x), static_cast<GLsizei>(m_viewportSize.y));
}

void Renderer::initialize(const std::filesystem::path& assetRoot)
{
    m_assetRoot = assetRoot;
    m_lighting = LightingEnvironment::createDemo();

    if (!gl::load()) {
        throw std::runtime_error("Failed to load modern OpenGL entry points");
    }

    m_shader.loadFromFiles(m_assetRoot / "shaders" / "geometry.vert", m_assetRoot / "shaders" / "geometry.frag");
    m_shadowShader.loadFromFiles(m_assetRoot / "shaders" / "shadow_depth.vert", m_assetRoot / "shaders" / "shadow_depth.frag");
    m_partShadowShader.loadFromFiles(m_assetRoot / "shaders" / "shadow_depth_part.vert", m_assetRoot / "shaders" / "shadow_depth.frag");
    initializeShadowResources();

    m_floorMesh = Mesh::createPlane(30.f);
    m_partRenderer.initialize(m_scene, m_assetRoot, m_assetRoot / "maps" / "demo.json");

    if (!m_texture.loadFromFile(m_assetRoot / "textures" / "cube.png")) {
        m_texture.createCheckerboard(128, 128, 16);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    const EntityId floorEntity = m_scene.createEntity();
    Transform& floorTransform = m_scene.addTransform(floorEntity);
    floorTransform.position = {0.f, -2.05f, 0.f};
    floorTransform.scaleFactor = {1.f, 1.f, 1.f};
    floorTransform.rebuildLocalMatrix();
    MeshRenderer& floorRenderer = m_scene.addMeshRenderer(floorEntity, &m_floorMesh, &m_texture);
    floorRenderer.material.baseColor = {1.f, 1.f, 1.f};

    m_camera.setPosition({0.f, 3.4f, 10.5f});
    m_camera.setYaw(-1.57079633f);
    m_camera.setPitch(-0.16f);
}

void Renderer::reloadResources(const std::filesystem::path& assetRoot)
{
    m_assetRoot = assetRoot;
    m_lighting = LightingEnvironment::createDemo();

    if (!gl::load()) {
        throw std::runtime_error("Failed to load modern OpenGL entry points");
    }

    m_shader.loadFromFiles(m_assetRoot / "shaders" / "geometry.vert", m_assetRoot / "shaders" / "geometry.frag");
    m_shadowShader.loadFromFiles(m_assetRoot / "shaders" / "shadow_depth.vert", m_assetRoot / "shaders" / "shadow_depth.frag");
    m_partShadowShader.loadFromFiles(m_assetRoot / "shaders" / "shadow_depth_part.vert", m_assetRoot / "shaders" / "shadow_depth.frag");
    initializeShadowResources();

    m_floorMesh.reload();
    m_partRenderer.reloadResources(m_assetRoot);

    if (!m_texture.loadFromFile(m_assetRoot / "textures" / "cube.png")) {
        m_texture.createCheckerboard(128, 128, 16);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Renderer::resize(sf::Vector2u size)
{
    if (size.x == 0u || size.y == 0u) {
        return;
    }

    m_viewportSize = size;
    m_camera.setAspectRatio(static_cast<float>(size.x) / static_cast<float>(size.y));
    glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
}

void Renderer::update(float /*deltaTime*/, float /*timeSeconds*/)
{
    m_scene.updateTransforms();
}

void Renderer::render(float timeSeconds)
{
    const float clearRed = 0.06f + 0.03f * std::sin(timeSeconds * 0.8f);
    const float clearGreen = 0.08f + 0.03f * std::sin(timeSeconds * 1.1f + 1.0f);
    const float clearBlue = 0.12f + 0.04f * std::sin(timeSeconds * 1.5f + 2.0f);

    glClearColor(clearRed, clearGreen, clearBlue, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const Mat4 view = m_camera.viewMatrix();
    const Mat4 projection = m_camera.projectionMatrix();
    const Mat4 lightSpace = buildLightSpaceMatrix();
    renderShadowPass(lightSpace, timeSeconds);

    const Vec3 cameraPosition = m_camera.position();
    m_partRenderer.render(view, projection, lightSpace, cameraPosition, m_lighting, m_shadowDepthTexture, timeSeconds);

    m_shader.bind();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    m_shader.setMat4("uLightSpace", lightSpace);
    m_shader.setVec3("uCameraPosition", cameraPosition);
    m_lighting.apply(m_shader);
    m_shader.setInt("uTexture", 0);
    m_shader.setInt("uShadowMap", 1);
    m_shader.setVec3("uBaseColor", {1.f, 1.f, 1.f});
    m_texture.bind(0);
    gl::activeTexture(GL_TEXTURE0 + 1);
    gl::bindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);

    m_scene.forEachRenderable([&](const Entity&, const Transform& transform, const MeshRenderer& meshRenderer) {
        m_shader.setMat4("uModel", transform.worldMatrix);
        m_shader.setVec3("uBaseColor", meshRenderer.material.baseColor);
        m_shader.setFloat("uMaterialRoughness", meshRenderer.material.roughness);
        m_shader.setFloat("uMaterialMetallic", meshRenderer.material.metallic);
        meshRenderer.mesh->draw();
    });
    m_shader.unbind();
}

} // namespace plunger
