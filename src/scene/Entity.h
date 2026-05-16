#pragma once

#include <cstdint>

namespace plunger {

using EntityId = std::uint32_t;

inline constexpr EntityId InvalidEntity = 0;

enum class ComponentMask : std::uint32_t {
    None = 0,
    Transform = 1u << 0,
    MeshRenderer = 1u << 1,
    Part = 1u << 2,
};

inline constexpr ComponentMask operator|(ComponentMask left, ComponentMask right)
{
    return static_cast<ComponentMask>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

inline constexpr ComponentMask operator&(ComponentMask left, ComponentMask right)
{
    return static_cast<ComponentMask>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

inline constexpr bool hasMask(std::uint32_t mask, ComponentMask required)
{
    return (mask & static_cast<std::uint32_t>(required)) == static_cast<std::uint32_t>(required);
}

struct Entity {
    EntityId id = InvalidEntity;
    std::uint32_t mask = 0;
};

} // namespace plunger