#pragma once

#include <filesystem>

#include <SFML/OpenGL.hpp>

namespace plunger {

class Texture {
public:
    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    bool loadFromFile(const std::filesystem::path& path);
    void createCheckerboard(unsigned int width, unsigned int height, unsigned int cellSize);
    void bind(unsigned int unit = 0) const;

    GLuint id() const
    {
        return m_textureId;
    }

private:
    void upload(const unsigned char* pixels, unsigned int width, unsigned int height);

    GLuint m_textureId = 0;
};

} // namespace plunger