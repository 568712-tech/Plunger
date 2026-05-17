#include "input/Input.h"

namespace plunger {

void Input::beginFrame(sf::Window& window)
{
    m_mouseDelta = {0.f, 0.f};

    for (std::size_t index = 0; index < static_cast<std::size_t>(sf::Keyboard::KeyCount); ++index) {
        m_keys[index] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(index));
    }

    updateMouseCapture(window);

    if (m_mouseCaptured) {
        const sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        if (m_firstMouseSample) {
            m_lastMousePosition = mousePosition;
            m_firstMouseSample = false;
        } else {
            m_mouseDelta = {
                static_cast<float>(mousePosition.x - m_lastMousePosition.x),
                static_cast<float>(mousePosition.y - m_lastMousePosition.y),
            };
        }
        const sf::Vector2u windowSize = window.getSize();
        const sf::Vector2i center = {
            static_cast<int>(windowSize.x / 2),
            static_cast<int>(windowSize.y / 2)
        };
        sf::Mouse::setPosition(center, window);
        m_lastMousePosition = center;
    } else {
        m_firstMouseSample = true;
    }
}

void Input::processEvent(const sf::Event& event)
{
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        m_keys[static_cast<std::size_t>(keyPressed->code)] = true;
    } else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
        m_keys[static_cast<std::size_t>(keyReleased->code)] = false;
    } else if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            m_mouseCaptured = !m_mouseCaptured;
            m_firstMouseSample = true;
        }
    }
}

bool Input::isKeyDown(sf::Keyboard::Key key) const
{
    const std::size_t index = static_cast<std::size_t>(key);
    if (index >= static_cast<std::size_t>(sf::Keyboard::KeyCount)) {
        return false;
    }

    return m_keys[index];
}

Vec2 Input::mouseDelta() const
{
    return m_mouseDelta;
}

void Input::resetMouseState(sf::Window& window)
{
    if (m_mouseCaptured) {
        updateMouseCapture(window);
        const sf::Vector2u windowSize = window.getSize();
        const sf::Vector2i center = {
            static_cast<int>(windowSize.x / 2),
            static_cast<int>(windowSize.y / 2)
        };
        sf::Mouse::setPosition(center, window);
        m_lastMousePosition = center;
        m_firstMouseSample = false;
        m_mouseDelta = {0.f, 0.f};
    } else {
        m_firstMouseSample = true;
    }
}

void Input::updateMouseCapture(sf::Window& window)
{
    window.setMouseCursorVisible(!m_mouseCaptured);
    window.setMouseCursorGrabbed(m_mouseCaptured);
}

} // namespace plunger
