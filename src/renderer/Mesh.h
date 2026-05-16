#pragma once

#include <SFML/OpenGL.hpp>

#include <cstddef>
#include <vector>

#include "renderer/Math.h"

namespace plunger {

class Mesh {
public:
    struct Vertex {
        Vec3 position;
        Vec3 normal;
        Vec2 texCoord;
    };

    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    static Mesh createCube();
    static Mesh createPlane(float size = 10.f);
    void draw() const;

private:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    void upload();

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    GLuint m_vertexArray = 0;
    GLuint m_vertexBuffer = 0;
    GLuint m_indexBuffer = 0;
};

} // namespace plunger