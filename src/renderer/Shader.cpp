#include "renderer/Shader.h"

#include "renderer/OpenGLFunctions.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace plunger {

Shader::Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
{
    loadFromFiles(vertexPath, fragmentPath);
}

Shader::~Shader()
{
    if (m_programId != 0) {
        gl::deleteProgram(m_programId);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_programId(other.m_programId)
{
    other.m_programId = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other) {
        if (m_programId != 0) {
            gl::deleteProgram(m_programId);
        }
        m_programId = other.m_programId;
        other.m_programId = 0;
    }

    return *this;
}

void Shader::loadFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
{
    const std::string vertexSource = readFile(vertexPath);
    const std::string fragmentSource = readFile(fragmentPath);

    const GLuint vertexShader = compileStage(GL_VERTEX_SHADER, vertexSource, vertexPath);
    const GLuint fragmentShader = compileStage(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
    const GLuint program = linkProgram(vertexShader, fragmentShader);

    gl::deleteShader(vertexShader);
    gl::deleteShader(fragmentShader);

    if (m_programId != 0) {
        gl::deleteProgram(m_programId);
    }

    m_programId = program;
}

void Shader::bind() const
{
    gl::useProgram(m_programId);
}

void Shader::unbind() const
{
    gl::useProgram(0);
}

void Shader::setMat4(const char* name, const Mat4& matrix) const
{
    gl::uniformMatrix4fv(gl::getUniformLocation(m_programId, name), 1, GL_FALSE, matrix.data());
}

void Shader::setVec3(const char* name, const Vec3& value) const
{
    gl::uniform3f(gl::getUniformLocation(m_programId, name), value.x, value.y, value.z);
}

void Shader::setFloat(const char* name, float value) const
{
    gl::uniform1f(gl::getUniformLocation(m_programId, name), value);
}

void Shader::setInt(const char* name, int value) const
{
    gl::uniform1i(gl::getUniformLocation(m_programId, name), value);
}

std::string Shader::readFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open shader file: " + path.string());
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

GLuint Shader::compileStage(GLenum stage, const std::string& source, const std::filesystem::path& path)
{
    const GLuint shader = gl::createShader(stage);
    const char* sourcePointer = source.c_str();
    gl::shaderSource(shader, 1, &sourcePointer, nullptr);
    gl::compileShader(shader);

    GLint compiled = GL_FALSE;
    gl::getShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint logLength = 0;
        gl::getShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::string log(static_cast<std::size_t>(logLength), '\0');
        gl::getShaderInfoLog(shader, logLength, nullptr, log.data());
        gl::deleteShader(shader);

        throw std::runtime_error("Failed to compile shader " + path.string() + "\n" + log);
    }

    return shader;
}

GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    const GLuint program = gl::createProgram();
    gl::attachShader(program, vertexShader);
    gl::attachShader(program, fragmentShader);
    gl::linkProgram(program);

    GLint linked = GL_FALSE;
    gl::getProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        GLint logLength = 0;
        gl::getProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::string log(static_cast<std::size_t>(logLength), '\0');
        gl::getProgramInfoLog(program, logLength, nullptr, log.data());
        gl::deleteProgram(program);

        throw std::runtime_error("Failed to link shader program\n" + log);
    }

    return program;
}

} // namespace plunger