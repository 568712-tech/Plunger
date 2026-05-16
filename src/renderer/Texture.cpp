#include "renderer/Texture.h"

#include "renderer/OpenGLFunctions.h"

#include <SFML/Graphics/Image.hpp>

#include <vector>

namespace plunger {

Texture::~Texture()
{
    if (m_textureId != 0) {
        gl::deleteTextures(1, &m_textureId);
    }
}

Texture::Texture(Texture&& other) noexcept
    : m_textureId(other.m_textureId)
{
    other.m_textureId = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        if (m_textureId != 0) {
            gl::deleteTextures(1, &m_textureId);
        }
        m_textureId = other.m_textureId;
        other.m_textureId = 0;
    }

    return *this;
}

bool Texture::loadFromFile(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        return false;
    }

    sf::Image image;
    if (!image.loadFromFile(path.string())) {
        return false;
    }

    const sf::Vector2u size = image.getSize();
    upload(image.getPixelsPtr(), size.x, size.y);
    return true;
}

void Texture::createCheckerboard(unsigned int width, unsigned int height, unsigned int cellSize)
{
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 4u);
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            const bool darkCell = ((x / cellSize) + (y / cellSize)) % 2u == 0u;
            const unsigned char value = darkCell ? 220u : 80u;
            const std::size_t index = (static_cast<std::size_t>(y) * width + x) * 4u;
            pixels[index + 0] = value;
            pixels[index + 1] = darkCell ? 180u : 120u;
            pixels[index + 2] = darkCell ? 255u : 180u;
            pixels[index + 3] = 255u;
        }
    }

    upload(pixels.data(), width, height);
}

void Texture::bind(unsigned int unit) const
{
    gl::activeTexture(GL_TEXTURE0 + unit);
    gl::bindTexture(GL_TEXTURE_2D, m_textureId);
}

void Texture::upload(const unsigned char* pixels, unsigned int width, unsigned int height)
{
    if (m_textureId == 0) {
        gl::genTextures(1, &m_textureId);
    }

    gl::bindTexture(GL_TEXTURE_2D, m_textureId);
    gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl::texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gl::texImage2D(GL_TEXTURE_2D,
        0,
        GL_RGBA,
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels);
    gl::generateMipmap(GL_TEXTURE_2D);
    gl::bindTexture(GL_TEXTURE_2D, 0);
}

} // namespace plunger