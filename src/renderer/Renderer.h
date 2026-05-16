#pragma once

#include "renderer/Camera.h"
#include "renderer/Mesh.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"

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
    Mesh m_cubeMesh;
    Mesh m_floorMesh;
    Texture m_texture;
    Camera m_camera;
    sf::Vector2u m_viewportSize {1u, 1u};
};

} // namespace plunger