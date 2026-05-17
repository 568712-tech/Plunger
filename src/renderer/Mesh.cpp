#include "renderer/Mesh.h"

#include "renderer/OpenGLFunctions.h"

#include <utility>

namespace plunger {

Mesh::~Mesh()
{
    if (m_indexBuffer != 0) {
        gl::deleteBuffers(1, &m_indexBuffer);
    }
    if (m_vertexBuffer != 0) {
        gl::deleteBuffers(1, &m_vertexBuffer);
    }
    if (m_vertexArray != 0) {
        gl::deleteVertexArrays(1, &m_vertexArray);
    }
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_vertices(std::move(other.m_vertices))
    , m_indices(std::move(other.m_indices))
    , m_vertexArray(other.m_vertexArray)
    , m_vertexBuffer(other.m_vertexBuffer)
    , m_indexBuffer(other.m_indexBuffer)
{
    other.m_vertexArray = 0;
    other.m_vertexBuffer = 0;
    other.m_indexBuffer = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other) {
        if (m_indexBuffer != 0) {
            gl::deleteBuffers(1, &m_indexBuffer);
        }
        if (m_vertexBuffer != 0) {
            gl::deleteBuffers(1, &m_vertexBuffer);
        }
        if (m_vertexArray != 0) {
            gl::deleteVertexArrays(1, &m_vertexArray);
        }
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_vertexArray = other.m_vertexArray;
        m_vertexBuffer = other.m_vertexBuffer;
        m_indexBuffer = other.m_indexBuffer;
        other.m_vertexArray = 0;
        other.m_vertexBuffer = 0;
        other.m_indexBuffer = 0;
    }

    return *this;
}

Mesh Mesh::createCube()
{
    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f,  0.5f}, {0.f, 0.f, 1.f}, {0.f, 0.f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.f, 0.f, 1.f}, {1.f, 0.f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f}, {1.f, 1.f}},
        {{-0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f}, {0.f, 1.f}},

        {{ 0.5f, -0.5f, -0.5f}, {0.f, 0.f,-1.f}, {0.f, 0.f}},
        {{-0.5f, -0.5f, -0.5f}, {0.f, 0.f,-1.f}, {1.f, 0.f}},
        {{-0.5f,  0.5f, -0.5f}, {0.f, 0.f,-1.f}, {1.f, 1.f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.f, 0.f,-1.f}, {0.f, 1.f}},

        {{-0.5f, -0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 0.f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.f, 0.f, 0.f}, {1.f, 0.f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.f, 0.f, 0.f}, {1.f, 1.f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 1.f}},

        {{ 0.5f, -0.5f,  0.5f}, {1.f, 0.f, 0.f}, {0.f, 0.f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 0.f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 1.f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.f, 0.f, 0.f}, {0.f, 1.f}},

        {{-0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f}, {1.f, 0.f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f}, {1.f, 1.f}},
        {{-0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f}, {0.f, 1.f}},

        {{-0.5f, -0.5f, -0.5f}, {0.f,-1.f, 0.f}, {0.f, 0.f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.f,-1.f, 0.f}, {1.f, 0.f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.f,-1.f, 0.f}, {1.f, 1.f}},
        {{-0.5f, -0.5f,  0.5f}, {0.f,-1.f, 0.f}, {0.f, 1.f}},
    };

    const std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };

    return Mesh::fromData(std::move(vertices), std::move(indices));
}

Mesh Mesh::createPlane(float size)
{
    const float halfSize = size * 0.5f;
    const std::vector<Vertex> vertices = {
        {{-halfSize, 0.f, -halfSize}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
        {{ halfSize, 0.f, -halfSize}, {0.f, 1.f, 0.f}, {1.f, 0.f}},
        {{ halfSize, 0.f,  halfSize}, {0.f, 1.f, 0.f}, {1.f, 1.f}},
        {{-halfSize, 0.f,  halfSize}, {0.f, 1.f, 0.f}, {0.f, 1.f}},
    };

    const std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};
    return Mesh::fromData(std::move(vertices), std::move(indices));
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
    : m_vertices(std::move(vertices))
    , m_indices(std::move(indices))
{
    upload();
}

Mesh Mesh::fromData(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    return Mesh(std::move(vertices), std::move(indices));
}

void Mesh::upload()
{
    gl::genVertexArrays(1, &m_vertexArray);
    gl::genBuffers(1, &m_vertexBuffer);
    gl::genBuffers(1, &m_indexBuffer);

    gl::bindVertexArray(m_vertexArray);

    gl::bindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    gl::bufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex)),
        m_vertices.data(),
        GL_STATIC_DRAW);

    gl::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    gl::bufferData(GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_indices.size() * sizeof(unsigned int)),
        m_indices.data(),
        GL_STATIC_DRAW);

    gl::enableVertexAttribArray(0);
    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    gl::enableVertexAttribArray(1);
    gl::vertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    gl::enableVertexAttribArray(2);
    gl::vertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

    gl::bindVertexArray(0);
}

void Mesh::reload()
{
    if (m_vertexArray != 0) {
        gl::deleteVertexArrays(1, &m_vertexArray);
        m_vertexArray = 0;
    }
    if (m_vertexBuffer != 0) {
        gl::deleteBuffers(1, &m_vertexBuffer);
        m_vertexBuffer = 0;
    }
    if (m_indexBuffer != 0) {
        gl::deleteBuffers(1, &m_indexBuffer);
        m_indexBuffer = 0;
    }

    upload();
}

void Mesh::draw() const
{
    gl::bindVertexArray(m_vertexArray);
    gl::drawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    gl::bindVertexArray(0);
}

} // namespace plunger