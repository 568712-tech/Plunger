#pragma once

#include "renderer/Camera.h"
#include "renderer/Mesh.h"
#include "renderer/PartRenderer.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "scene/Scene.h"

#include <filesystem>

#include <SFML/OpenGL.hpp>
#include <SFML/Window/Window.hpp>

namespace plunger {

class Renderer {
public:
    void initialize(const std::filesystem::path& assetRoot);
    void resize(sf::Vector2u size);
    void render(float timeSeconds);

private:
    std::filesystem::path m_assetRoot;
    Shader m_shader;
    Mesh m_modelMesh;
    Mesh m_floorMesh;
    PartRenderer m_partRenderer;
    Texture m_texture;
    Camera m_camera;
    Scene m_scene;
    Material m_modelMaterial;
    EntityId m_floorEntity = InvalidEntity;
    EntityId m_centerCubeEntity = InvalidEntity;
    EntityId m_satelliteCubeEntity = InvalidEntity;
    sf::Vector2u m_viewportSize {1u, 1u};
};

} 