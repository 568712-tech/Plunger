#pragma once

#include "renderer/Math.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Window.hpp>

namespace plunger {

class Input {
public:
    void beginFrame(sf::Window& window);
    void processEvent(const sf::Event& event);

    bool isKeyDown(sf::Keyboard::Key key) const;
    bool isKeyPressed(sf::Keyboard::Key key) const;
    Vec2 mouseDelta() const;
    bool mouseCaptured() const
    {
        return m_mouseCaptured;
    }

    void resetMouseState(sf::Window& window);

private:
    void updateMouseCapture(sf::Window& window);

    bool m_keys[static_cast<std::size_t>(sf::Keyboard::KeyCount)] {};
    bool m_keysPressed[static_cast<std::size_t>(sf::Keyboard::KeyCount)] {};
    Vec2 m_mouseDelta {0.f, 0.f};
    bool m_mouseCaptured = false;
    bool m_firstMouseSample = true;
    sf::Vector2i m_lastMousePosition {0, 0};
};

} // namespace plunger
