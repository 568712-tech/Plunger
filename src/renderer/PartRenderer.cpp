#include "renderer/PartRenderer.h"

#include "renderer/OpenGLFunctions.h"
#include "scene/PartMapLoader.h"

#include <cstddef>
#include <array>
#include <cmath>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace plunger {
namespace {

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texCoord;
};

} // namespace

void PartRenderer::initialize(Scene& scene, const std::filesystem::path& assetRoot, const std::filesystem::path& mapPath)
{
    m_scene = &scene;

    const std::vector<Part> parts = PartMapLoader::loadJson(mapPath);
    for (const Part& part : parts) {
        const EntityId entityId = m_scene->createEntity();
        m_scene->addPart(entityId, part);
    }

    m_instances.resize(parts.size());

    uploadGeometry();

    m_shader.loadFromFiles(assetRoot / "shaders" / "part.vert", assetRoot / "shaders" / "part.frag");
}

void PartRenderer::renderShadowPass(Shader& shadowShader, const Mat4& lightSpace, float timeSeconds) const
{
    uploadInstances(timeSeconds);

    shadowShader.bind();
    shadowShader.setMat4("uLightSpace", lightSpace);

    gl::bindVertexArray(m_vertexArray);
    gl::drawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(m_instances.size()));
    gl::bindVertexArray(0);

    shadowShader.unbind();
}

void PartRenderer::render(const Mat4& view,
    const Mat4& projection,
    const Mat4& lightSpace,
    const Vec3& cameraPosition,
    const LightingEnvironment& lighting,
    GLuint shadowMapTexture,
    float timeSeconds) const
{
    uploadInstances(timeSeconds);

    m_shader.bind();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    m_shader.setMat4("uLightSpace", lightSpace);
    m_shader.setVec3("uCameraPosition", cameraPosition);
    lighting.apply(m_shader);
    m_shader.setFloat("uMaterialRoughness", 0.85f);
    m_shader.setFloat("uMaterialMetallic", 0.05f);
    m_shader.setInt("uShadowMap", 1);
    gl::activeTexture(GL_TEXTURE0 + 1);
    gl::bindTexture(GL_TEXTURE_2D, shadowMapTexture);

    gl::bindVertexArray(m_vertexArray);
    gl::drawElementsInstanced(GL_TRIANGLES,
        36,
        GL_UNSIGNED_INT,
        nullptr,
        static_cast<GLsizei>(m_instances.size()));

    gl::bindVertexArray(0);
    m_shader.unbind();
}

void PartRenderer::reloadResources(const std::filesystem::path& assetRoot)
{
    m_shader.loadFromFiles(assetRoot / "shaders" / "part.vert", assetRoot / "shaders" / "part.frag");
    releaseGeometry();
    uploadGeometry();
}

void PartRenderer::releaseGeometry()
{
    if (m_instanceBuffer != 0) {
        gl::deleteBuffers(1, &m_instanceBuffer);
        m_instanceBuffer = 0;
    }
    if (m_indexBuffer != 0) {
        gl::deleteBuffers(1, &m_indexBuffer);
        m_indexBuffer = 0;
    }
    if (m_vertexBuffer != 0) {
        gl::deleteBuffers(1, &m_vertexBuffer);
        m_vertexBuffer = 0;
    }
    if (m_vertexArray != 0) {
        gl::deleteVertexArrays(1, &m_vertexArray);
        m_vertexArray = 0;
    }
}

void PartRenderer::uploadGeometry()
{
    const std::array<Vertex, 24> vertices = {
        Vertex{{-0.5f, -0.5f,  0.5f}, {0.f, 0.f, 1.f}, {0.f, 0.f}},
        Vertex{{ 0.5f, -0.5f,  0.5f}, {0.f, 0.f, 1.f}, {1.f, 0.f}},
        Vertex{{ 0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f}, {1.f, 1.f}},
        Vertex{{-0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f}, {0.f, 1.f}},

        Vertex{{ 0.5f, -0.5f, -0.5f}, {0.f, 0.f,-1.f}, {0.f, 0.f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, {0.f, 0.f,-1.f}, {1.f, 0.f}},
        Vertex{{-0.5f,  0.5f, -0.5f}, {0.f, 0.f,-1.f}, {1.f, 1.f}},
        Vertex{{ 0.5f,  0.5f, -0.5f}, {0.f, 0.f,-1.f}, {0.f, 1.f}},

        Vertex{{-0.5f, -0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 0.f}},
        Vertex{{-0.5f, -0.5f,  0.5f}, {-1.f, 0.f, 0.f}, {1.f, 0.f}},
        Vertex{{-0.5f,  0.5f,  0.5f}, {-1.f, 0.f, 0.f}, {1.f, 1.f}},
        Vertex{{-0.5f,  0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 1.f}},

        Vertex{{ 0.5f, -0.5f,  0.5f}, {1.f, 0.f, 0.f}, {0.f, 0.f}},
        Vertex{{ 0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 0.f}},
        Vertex{{ 0.5f,  0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 1.f}},
        Vertex{{ 0.5f,  0.5f,  0.5f}, {1.f, 0.f, 0.f}, {0.f, 1.f}},

        Vertex{{-0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
        Vertex{{ 0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f}, {1.f, 0.f}},
        Vertex{{ 0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f}, {1.f, 1.f}},
        Vertex{{-0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f}, {0.f, 1.f}},

        Vertex{{-0.5f, -0.5f, -0.5f}, {0.f,-1.f, 0.f}, {0.f, 0.f}},
        Vertex{{ 0.5f, -0.5f, -0.5f}, {0.f,-1.f, 0.f}, {1.f, 0.f}},
        Vertex{{ 0.5f, -0.5f,  0.5f}, {0.f,-1.f, 0.f}, {1.f, 1.f}},
        Vertex{{-0.5f, -0.5f,  0.5f}, {0.f,-1.f, 0.f}, {0.f, 1.f}},
    };

    const std::array<unsigned int, 36> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };

    gl::genVertexArrays(1, &m_vertexArray);
    gl::genBuffers(1, &m_vertexBuffer);
    gl::genBuffers(1, &m_indexBuffer);
    gl::genBuffers(1, &m_instanceBuffer);

    gl::bindVertexArray(m_vertexArray);

    gl::bindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    gl::bufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STATIC_DRAW);

    gl::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    gl::bufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);

    gl::enableVertexAttribArray(0);
    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    gl::enableVertexAttribArray(1);
    gl::vertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    gl::enableVertexAttribArray(2);
    gl::vertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

    gl::bindBuffer(GL_ARRAY_BUFFER, m_instanceBuffer);
    gl::bufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_instances.size() * sizeof(InstanceData)),
        m_instances.data(),
        GL_DYNAMIC_DRAW);

    const std::size_t matrixStride = sizeof(InstanceData);
    for (GLuint column = 0; column < 4; ++column) {
        const GLuint location = 3u + column;
        gl::enableVertexAttribArray(location);
        gl::vertexAttribPointer(location,
            4,
            GL_FLOAT,
            GL_FALSE,
            static_cast<GLsizei>(matrixStride),
            reinterpret_cast<void*>(column * 4u * sizeof(float)));
        gl::vertexAttribDivisor(location, 1);
    }

    gl::enableVertexAttribArray(7);
    gl::vertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(matrixStride), reinterpret_cast<void*>(sizeof(Mat4)));
    gl::vertexAttribDivisor(7, 1);

    gl::bindVertexArray(0);
}

void PartRenderer::uploadInstances(float timeSeconds) const
{
    if (m_scene == nullptr) {
        return;
    }

    std::size_t partCount = 0;
    m_scene->forEachPart([&](const Entity&, const Part&) {
        ++partCount;
    });

    m_instances.resize(partCount);

    std::size_t index = 0;
    m_scene->forEachPart([&](const Entity&, const Part& part) {
        const float pulse = 0.03f * std::sin(timeSeconds * 1.2f + static_cast<float>(index) * 0.35f);
        InstanceData& instance = m_instances[index];
        instance.model = multiply(translate({part.position.x, part.position.y + pulse, part.position.z}), scale(part.size));
        instance.color = part.material.baseColor;
        instance.padding = 0.f;
        ++index;
    });

    gl::bindBuffer(GL_ARRAY_BUFFER, m_instanceBuffer);
    gl::bufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_instances.size() * sizeof(InstanceData)),
        m_instances.data(),
        GL_DYNAMIC_DRAW);
}

} // namespace plunger
