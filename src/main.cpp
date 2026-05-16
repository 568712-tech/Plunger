#include <SFML/OpenGL.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window.hpp>

#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "renderer/Renderer.h"

namespace {

const char* glString(GLenum name)
{
    const GLubyte* value = glGetString(name);
    return value ? reinterpret_cast<const char*>(value) : "unavailable";
}

class Engine {
public:
    Engine()
    {
        sf::ContextSettings settings;
        settings.depthBits = 24;
        settings.stencilBits = 8;
        settings.antiAliasingLevel = 4;
        settings.majorVersion = 3;
        settings.minorVersion = 3;
        settings.attributeFlags = sf::ContextSettings::Attribute::Core;

        m_window.create(sf::VideoMode({1280u, 720u}), "Plunger", sf::State::Windowed, settings);
        m_window.setVerticalSyncEnabled(true);
        if (!m_window.setActive(true)) {
            throw std::runtime_error("Failed to activate OpenGL context");
        }

        m_renderer.initialize(std::filesystem::current_path() / "assets");
        m_renderer.resize(m_window.getSize());
        logContextInfo();
    }

    void run()
    {
        while (m_window.isOpen()) {
            processEvents();

            const float frameTime = m_clock.restart().asSeconds();
            m_accumulator += frameTime;
            m_simulationTime += frameTime;

            while (m_accumulator >= fixedDeltaTime) {
                update(fixedDeltaTime);
                m_accumulator -= fixedDeltaTime;
            }

            const float interpolation = m_accumulator / fixedDeltaTime;
            render(interpolation);
        }
    }

private:
    static constexpr float fixedDeltaTime = 1.f / 60.f;

    void processEvents()
    {
        while (const std::optional event = m_window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                m_window.close();
            } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                m_renderer.resize(resized->size);
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    m_window.close();
                }
            }
        }
    }

    void update(float deltaTime)
    {
        m_simulationTime += deltaTime;
    }

    void render(float interpolation)
    {
        m_renderer.render(m_simulationTime + interpolation * fixedDeltaTime);
        m_window.display();
    }

    void logContextInfo()
    {
        const auto settings = m_window.getSettings();

        std::cout << "Plunger engine booted\n";
        std::cout << "Context: OpenGL " << settings.majorVersion << '.' << settings.minorVersion << '\n';
        std::cout << "Depth bits: " << settings.depthBits << ", stencil bits: " << settings.stencilBits << '\n';
        std::cout << "Vendor: " << glString(GL_VENDOR) << '\n';
        std::cout << "Renderer: " << glString(GL_RENDERER) << '\n';
        std::cout << "Version: " << glString(GL_VERSION) << '\n';
    }

    sf::Window m_window;
    plunger::Renderer m_renderer;
    sf::Clock m_clock;
    float m_accumulator = 0.f;
    float m_simulationTime = 0.f;
};

} // namespace

int main()
{
    Engine engine;
    engine.run();
    return 0;
}