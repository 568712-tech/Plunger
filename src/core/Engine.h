#pragma once

#include "game/FlyCameraController.h"
#include "input/Input.h"
#include "renderer/Renderer.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Window.hpp>

#include <filesystem>

namespace plunger {

class Engine {
public:
    Engine();
    void run();

private:
    static constexpr float fixedDeltaTime = 1.f / 60.f;

    void processEvents();
    void update(float deltaTime);
    void render(float interpolation);
    void logContextInfo();
    void toggleFullscreen();

    sf::Window m_window;
    sf::ContextSettings m_contextSettings;
    sf::Vector2u m_windowedSize {1280u, 720u};
    Renderer m_renderer;
    Input m_input;
    FlyCameraController m_cameraController;
    sf::Clock m_clock;
    float m_accumulator = 0.f;
    float m_simulationTime = 0.f;
    bool m_isFullscreen = false;
    std::filesystem::path m_assetRoot;

};

} // namespace plunger
