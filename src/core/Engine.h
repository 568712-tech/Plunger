#pragma once

#include "game/FlyCameraController.h"
#include "input/Input.h"
#include "physics/PhysicsWorld.h"
#include "renderer/Renderer.h"
#include "game/Character.h"

#include <memory>

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
    void resetDemoRun();
    void updateWindowTitle();
    bool reachedGoal() const;
    void logContextInfo();
    void toggleFullscreen();
    void setupPhysicsColliders();

    sf::Window m_window;
    sf::ContextSettings m_contextSettings;
    sf::Vector2u m_windowedSize {1280u, 720u};
    Renderer m_renderer;
    PhysicsWorld m_physics;
    Input m_input;
    FlyCameraController m_cameraController;
    sf::Clock m_clock;
    float m_accumulator = 0.f;
    float m_simulationTime = 0.f;
    float m_finishTime = 0.f;
    bool m_isFullscreen = false;
    bool m_demoFinished = false;
    std::filesystem::path m_assetRoot;
    Vec3 m_spawnPosition {0.0f, 0.0f, -20.0f};
    float m_spawnYaw = 1.57079633f;
    float m_spawnPitch = 0.0f;
    Vec3 m_goalCenter {0.0f, 4.5f, 40.5f};
    Vec3 m_goalHalfExtents {2.0f, 2.0f, 2.0f};
    std::unique_ptr<Character> m_player;
    bool m_firstPersonMode = false;
};

} // namespace plunger
