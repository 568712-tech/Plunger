#pragma once

#include "renderer/Math.h"

#include <filesystem>
#include <string>

#include <SFML/OpenGL.hpp>

namespace plunger {

class Shader {
public:
    Shader() = default;
    Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void loadFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    void bind() const;
    void unbind() const;

    void setMat4(const char* name, const Mat4& matrix) const;
    void setVec3(const char* name, const Vec3& value) const;
    void setFloat(const char* name, float value) const;
    void setInt(const char* name, int value) const;

    GLuint id() const
    {
        return m_programId;
    }

private:
    static std::string readFile(const std::filesystem::path& path);
    static GLuint compileStage(GLenum stage, const std::string& source, const std::filesystem::path& path);
    static GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

    GLuint m_programId = 0;
};

} // namespace plunger