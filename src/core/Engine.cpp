#include "core/Engine.h"

#include <SFML/OpenGL.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace plunger {
namespace {

const char* glString(GLenum name)
{
    const GLubyte* value = glGetString(name);
    return value ? reinterpret_cast<const char*>(value) : "unavailable";
}

} // namespace

Engine::Engine()
{
    m_contextSettings.depthBits = 24;
    m_contextSettings.stencilBits = 8;
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
            }
        }
    }

    m_input.beginFrame(m_window);
}

void Engine::update(float deltaTime)
{
    m_simulationTime += deltaTime;
    m_cameraController.update(m_renderer.camera(), m_input, deltaTime);
    m_renderer.update(deltaTime, m_simulationTime);
}

void Engine::render(float interpolation)
{
    const float renderTime = m_simulationTime + interpolation * fixedDeltaTime;
    m_renderer.render(renderTime);
    m_window.display();
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
    std::cout << "Controls: WASD move, Space up, Shift down, LMB look, F11 toggle fullscreen\n";
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
}

} // namespace plunger
