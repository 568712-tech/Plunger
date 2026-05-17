#include "core/Engine.h"

#include <SFML/OpenGL.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>

#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace plunger {
namespace {

const char* glString(GLenum name)
{
    const GLubyte* value = glGetString(name);
    return value ? reinterpret_cast<const char*>(value) : "unavailable";
}

bool insideAabb(const Vec3& point, const Vec3& center, const Vec3& halfExtents)
{
    return std::abs(point.x - center.x) <= halfExtents.x
        && std::abs(point.y - center.y) <= halfExtents.y
        && std::abs(point.z - center.z) <= halfExtents.z;
}

} // namespace

Engine::Engine()
{
    m_contextSettings.depthBits = 24;
    // m_contextSettings.stencilBits = 8;
    m_contextSettings.antiAliasingLevel = 4;
    m_contextSettings.majorVersion = 3;
    m_contextSettings.minorVersion = 3;
    m_contextSettings.attributeFlags = sf::ContextSettings::Attribute::Core;

    m_window.create(sf::VideoMode(sf::Vector2u{1280u, 720u}, 32u), "Plunger", sf::State::Windowed, m_contextSettings);
    m_window.setVerticalSyncEnabled(true);
    if (!m_window.setActive(true)) {
        throw std::runtime_error("Failed to activate OpenGL context");
    }

    m_assetRoot = std::filesystem::current_path() / "assets";
    m_renderer.initialize(m_assetRoot);
    resetDemoRun();
    // Create player character after renderer and scene are initialized
    m_player = std::make_unique<Character>(m_renderer.scene(), m_spawnPosition, 0.0f);
    m_renderer.resize(m_window.getSize());
    logContextInfo();
}

void Engine::run()
{
    while (m_window.isOpen()) {
        processEvents();

        const float frameTime = m_clock.restart().asSeconds();
        m_accumulator += frameTime;

        while (m_accumulator >= fixedDeltaTime) {
            update(fixedDeltaTime);
            m_accumulator -= fixedDeltaTime;
        }

        const float interpolation = m_accumulator / fixedDeltaTime;
        render(interpolation);
    }
}

void Engine::processEvents()
{
    while (const std::optional event = m_window.pollEvent()) {
        m_input.processEvent(*event);

        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            m_renderer.resize(resized->size);
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                m_window.close();
            } else if (keyPressed->code == sf::Keyboard::Key::F11) {
                toggleFullscreen();
            } else if (keyPressed->code == sf::Keyboard::Key::R) {
                resetDemoRun();
            } else if (keyPressed->code == sf::Keyboard::Key::F5) {
                m_firstPersonMode = !m_firstPersonMode;
                if (m_player) {
                    m_player->setVisible(!m_firstPersonMode);
                }
            }
        }
    }

    m_input.beginFrame(m_window);
}

void Engine::update(float deltaTime)
{
    m_simulationTime += deltaTime;
    m_cameraController.update(m_renderer.camera(), m_input, deltaTime);
    if (m_player) {
        m_player->update(deltaTime, m_simulationTime, m_input, m_renderer.camera());
    }
    m_renderer.update(deltaTime, m_simulationTime);

    // Camera positioning: first-person or third-person follow camera
    if (m_player) {
        const Vec3 playerPos = m_player->position();
        Camera& camera = m_renderer.camera();

        if (m_firstPersonMode) {
            // First-person: camera at player position (at head height)
            const float headHeight = 1.6f;
            camera.setPosition({playerPos.x, playerPos.y + headHeight, playerPos.z});
        } else {
            // Third-person: position the camera behind the player and look at them
            const float followDistance = 3.0f;
            const float followHeight = 2.0f;
            Vec3 camForward = camera.forward();
            camForward.y = 0.f;
            const float len = std::sqrt(camForward.x * camForward.x + camForward.z * camForward.z);
            if (len < 1e-5f) {
                camForward = {0.f, 0.f, -1.f};
            } else {
                camForward.x /= len;
                camForward.z /= len;
            }
            Vec3 desiredPos = {playerPos.x - camForward.x * followDistance, playerPos.y + followHeight, playerPos.z - camForward.z * followDistance};
            camera.setPosition(desiredPos);
        }
    }

    if (!m_demoFinished && reachedGoal()) {
        m_demoFinished = true;
        m_finishTime = m_simulationTime;
    }

    updateWindowTitle();
}

void Engine::render(float interpolation)
{
    const float renderTime = m_simulationTime + interpolation * fixedDeltaTime;
    m_renderer.render(renderTime);
    m_window.display();
}

void Engine::resetDemoRun()
{
    Camera& camera = m_renderer.camera();
    camera.setPosition(m_spawnPosition);
    camera.setYaw(m_spawnYaw);
    camera.setPitch(m_spawnPitch);
    m_simulationTime = 0.f;
    m_finishTime = 0.f;
    m_accumulator = 0.f;
    m_demoFinished = false;
    if (m_player) {
        m_player->setPosition(m_spawnPosition);
    }
    updateWindowTitle();
}

void Engine::updateWindowTitle()
{
    std::ostringstream title;
    title << "Plunger Demo Run";

    if (m_demoFinished) {
        title << " | Finished in " << std::fixed << std::setprecision(2) << m_finishTime << "s";
    } else {
        title << " | Time " << std::fixed << std::setprecision(2) << m_simulationTime << "s";
    }

    title << " | Reach the glowing finish gate | R reset | F11 fullscreen | F5 toggle view (" << (m_firstPersonMode ? "1st" : "3rd") << " person)";
    m_window.setTitle(title.str());
}

bool Engine::reachedGoal() const
{
    return insideAabb(m_renderer.camera().position(), m_goalCenter, m_goalHalfExtents);
}

void Engine::logContextInfo()
{
    const auto settings = m_window.getSettings();

    std::cout << "Plunger engine booted\n";
    std::cout << "Context: OpenGL " << settings.majorVersion << '.' << settings.minorVersion << '\n';
    std::cout << "Depth bits: " << settings.depthBits << ", stencil bits: " << settings.stencilBits << '\n';
    std::cout << "Vendor: " << glString(GL_VENDOR) << '\n';
    std::cout << "Renderer: " << glString(GL_RENDERER) << '\n';
    std::cout << "Version: " << glString(GL_VERSION) << '\n';
    std::cout << "Controls: WASD move, Space up, Shift down, LMB look, R reset, F11 toggle fullscreen\n";
    std::cout << "Objective: fly from the launch pad to the glowing finish gate.\n";
}

void Engine::toggleFullscreen()
{
    m_isFullscreen = !m_isFullscreen;
    if (m_isFullscreen) {
        m_windowedSize = m_window.getSize();
        m_window.create(sf::VideoMode::getDesktopMode(), "Plunger", sf::State::Fullscreen, m_contextSettings);
    } else {
        m_window.create(sf::VideoMode(m_windowedSize, 32u), "Plunger", sf::State::Windowed, m_contextSettings);
    }

    m_window.setVerticalSyncEnabled(true);
    if (!m_window.setActive(true)) {
        throw std::runtime_error("Failed to activate OpenGL context");
    }

    m_input.resetMouseState(m_window);
    m_renderer.reloadResources(m_assetRoot);
    m_renderer.resize(m_window.getSize());
    updateWindowTitle();
}

} // namespace plunger
