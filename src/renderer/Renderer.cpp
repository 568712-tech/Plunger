#include "renderer/Renderer.h"

#include "renderer/OpenGLFunctions.h"
#include "scene/ModelLoader.h"

#include <cmath>
#include <filesystem>
#include <utility>
#include <stdexcept>

namespace plunger {

void Renderer::initialize(const std::filesystem::path& assetRoot)
{
    m_assetRoot = assetRoot;

    if (!gl::load()) {
        throw std::runtime_error("Failed to load modern OpenGL entry points");
    }

    m_shader.loadFromFiles(m_assetRoot / "shaders" / "geometry.vert", m_assetRoot / "shaders" / "geometry.frag");

    const std::filesystem::path gltfPath = m_assetRoot / "models" / "cube.gltf";
    if (std::filesystem::exists(gltfPath)) {
        GltfModel gltfModel = ModelLoader::loadGltf(gltfPath);
        m_modelMesh = std::move(gltfModel.mesh);
        if (!gltfModel.materials.empty()) {
            m_modelMaterial = gltfModel.materials.front().material;
        }
    } else {
        m_modelMesh = ModelLoader::loadObj(m_assetRoot / "models" / "cube.obj");
        m_modelMaterial.baseColor = {1.f, 1.f, 1.f};
    }
    m_floorMesh = Mesh::createPlane(14.f);
    m_partRenderer.initialize(m_scene, m_assetRoot / "maps" / "demo.json");

    if (!m_texture.loadFromFile(m_assetRoot / "textures" / "cube.png")) {
        m_texture.createCheckerboard(128, 128, 16);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_floorEntity = m_scene.createEntity();
    Transform& floorTransform = m_scene.addTransform(m_floorEntity);
    floorTransform.position = {0.f, -1.15f, 0.f};
    floorTransform.scaleFactor = {1.f, 1.f, 1.f};
    floorTransform.rebuildLocalMatrix();
    MeshRenderer& floorRenderer = m_scene.addMeshRenderer(m_floorEntity, &m_floorMesh, &m_texture);
    floorRenderer.material.baseColor = {1.f, 1.f, 1.f};

    m_centerCubeEntity = m_scene.createEntity();
    Transform& cubeTransform = m_scene.addTransform(m_centerCubeEntity);
    cubeTransform.position = {0.f, 0.5f, -6.f};
    cubeTransform.scaleFactor = {1.f, 1.f, 1.f};
    cubeTransform.rebuildLocalMatrix();
    MeshRenderer& centerRenderer = m_scene.addMeshRenderer(m_centerCubeEntity, &m_modelMesh, &m_texture);
    centerRenderer.material = m_modelMaterial;

    m_satelliteCubeEntity = m_scene.createEntity();
    Transform& satelliteTransform = m_scene.addTransform(m_satelliteCubeEntity);
    satelliteTransform.position = {2.8f, 1.0f, 0.f};
    satelliteTransform.scaleFactor = {0.7f, 0.7f, 0.7f};
    satelliteTransform.rebuildLocalMatrix();
    MeshRenderer& satelliteRenderer = m_scene.addMeshRenderer(m_satelliteCubeEntity, &m_modelMesh, &m_texture);
    satelliteRenderer.material = m_modelMaterial;

    m_scene.setParent(m_satelliteCubeEntity, m_centerCubeEntity);

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

    if (Transform* cube = m_scene.getTransform(m_centerCubeEntity); cube != nullptr) {
        cube->rotation = {timeSeconds * 0.45f, timeSeconds * 0.8f, 0.f};
        cube->rebuildLocalMatrix();
    }

    const Mat4 view = m_camera.viewMatrix();
    const Mat4 projection = m_camera.projectionMatrix();
    m_scene.updateTransforms();

    m_partRenderer.render(view, projection, timeSeconds);

    m_shader.bind();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    m_shader.setVec3("uLightDirection", normalize({-0.4f, -1.f, -0.25f}));
    m_shader.setInt("uTexture", 0);
    m_shader.setVec3("uBaseColor", {1.f, 1.f, 1.f});
    m_texture.bind(0);

    m_scene.forEachRenderable([&](const Entity&, const Transform& transform, const MeshRenderer& meshRenderer) {
        m_shader.setMat4("uModel", transform.worldMatrix);
        m_shader.setVec3("uBaseColor", meshRenderer.material.baseColor);
        meshRenderer.mesh->draw();
    });
    m_shader.unbind();
}

} // namespace plunger