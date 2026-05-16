#include "renderer/Renderer.h"

#include "renderer/OpenGLFunctions.h"

#include <cmath>
#include <filesystem>
#include <stdexcept>

namespace plunger {

void Renderer::initialize(const std::filesystem::path& assetRoot)
{
    m_assetRoot = assetRoot;

    if (!gl::load()) {
        throw std::runtime_error("Failed to load modern OpenGL entry points");
    }

    m_shader.loadFromFiles(m_assetRoot / "shaders" / "geometry.vert", m_assetRoot / "shaders" / "geometry.frag");
    m_cubeMesh = Mesh::createCube();
    m_floorMesh = Mesh::createPlane(14.f);

    if (!m_texture.loadFromFile(m_assetRoot / "textures" / "cube.png")) {
        m_texture.createCheckerboard(128, 128, 16);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_camera.setPosition({0.f, 1.6f, 5.5f});
    m_camera.setPitch(-0.1f);
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

void Renderer::render(float timeSeconds)
{
    const float orbitAngle = timeSeconds * 0.35f;
    m_camera.setPosition({std::cos(orbitAngle) * 7.0f, 2.6f, std::sin(orbitAngle) * 7.0f});
    m_camera.setYaw(orbitAngle + 3.14159265f);
    m_camera.setPitch(-0.28f);

    const float clearRed = 0.06f + 0.03f * std::sin(timeSeconds * 0.8f);
    const float clearGreen = 0.08f + 0.03f * std::sin(timeSeconds * 1.1f + 1.0f);
    const float clearBlue = 0.12f + 0.04f * std::sin(timeSeconds * 1.5f + 2.0f);

    glClearColor(clearRed, clearGreen, clearBlue, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const Mat4 model = multiply(rotateY(timeSeconds * 0.8f), rotateX(timeSeconds * 0.45f));
    const Mat4 view = m_camera.viewMatrix();
    const Mat4 projection = m_camera.projectionMatrix();

    m_shader.bind();
    m_shader.setMat4("uModel", model);
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    m_shader.setVec3("uLightDirection", normalize({-0.4f, -1.f, -0.25f}));
    m_shader.setInt("uTexture", 0);
    m_texture.bind(0);

    m_shader.setMat4("uModel", multiply(translate({0.f, -1.15f, 0.f}), scale({1.f, 1.f, 1.f})));
    m_floorMesh.draw();

    m_shader.setMat4("uModel", multiply(translate({0.f, 0.f, 0.f}), multiply(rotateY(timeSeconds * 0.8f), rotateX(timeSeconds * 0.45f))));
    m_cubeMesh.draw();

    m_shader.setMat4("uModel", multiply(translate({2.2f, -0.2f, -1.6f}), multiply(rotateY(-timeSeconds * 1.1f), scale({0.7f, 0.7f, 0.7f}))));
    m_cubeMesh.draw();
    m_shader.unbind();
}

} // namespace plunger