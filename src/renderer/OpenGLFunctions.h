#pragma once

#include <SFML/OpenGL.hpp>

#include <cstddef>

using GLchar = char;
using GLsizeiptr = std::ptrdiff_t;

#ifndef GL_VERTEX_SHADER
constexpr GLenum GL_VERTEX_SHADER = 0x8B31;
#endif
#ifndef GL_FRAGMENT_SHADER
constexpr GLenum GL_FRAGMENT_SHADER = 0x8B30;
#endif
#ifndef GL_COMPILE_STATUS
constexpr GLenum GL_COMPILE_STATUS = 0x8B81;
#endif
#ifndef GL_LINK_STATUS
constexpr GLenum GL_LINK_STATUS = 0x8B82;
#endif
#ifndef GL_INFO_LOG_LENGTH
constexpr GLenum GL_INFO_LOG_LENGTH = 0x8B84;
#endif
#ifndef GL_ARRAY_BUFFER
constexpr GLenum GL_ARRAY_BUFFER = 0x8892;
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
constexpr GLenum GL_ELEMENT_ARRAY_BUFFER = 0x8893;
#endif
#ifndef GL_STATIC_DRAW
constexpr GLenum GL_STATIC_DRAW = 0x88E4;
#endif
#ifndef GL_DYNAMIC_DRAW
constexpr GLenum GL_DYNAMIC_DRAW = 0x88E8;
#endif
#ifndef GL_FLOAT
constexpr GLenum GL_FLOAT = 0x1406;
#endif
#ifndef GL_TRIANGLES
constexpr GLenum GL_TRIANGLES = 0x0004;
#endif
#ifndef GL_UNSIGNED_INT
constexpr GLenum GL_UNSIGNED_INT = 0x1405;
#endif
#ifndef GL_TEXTURE_2D
constexpr GLenum GL_TEXTURE_2D = 0x0DE1;
#endif
#ifndef GL_TEXTURE0
constexpr GLenum GL_TEXTURE0 = 0x84C0;
#endif
#ifndef GL_TEXTURE_MIN_FILTER
constexpr GLenum GL_TEXTURE_MIN_FILTER = 0x2801;
#endif
#ifndef GL_TEXTURE_MAG_FILTER
constexpr GLenum GL_TEXTURE_MAG_FILTER = 0x2800;
#endif
#ifndef GL_TEXTURE_WRAP_S
constexpr GLenum GL_TEXTURE_WRAP_S = 0x2802;
#endif
#ifndef GL_TEXTURE_WRAP_T
constexpr GLenum GL_TEXTURE_WRAP_T = 0x2803;
#endif
#ifndef GL_LINEAR
constexpr GLenum GL_LINEAR = 0x2601;
#endif
#ifndef GL_LINEAR_MIPMAP_LINEAR
constexpr GLenum GL_LINEAR_MIPMAP_LINEAR = 0x2703;
#endif
#ifndef GL_REPEAT
constexpr GLenum GL_REPEAT = 0x2901;
#endif
#ifndef GL_RGBA
constexpr GLenum GL_RGBA = 0x1908;
#endif
#ifndef GL_UNSIGNED_BYTE
constexpr GLenum GL_UNSIGNED_BYTE = 0x1401;
#endif

namespace plunger::gl {

using CreateShaderProc = GLuint (*)(GLenum);
using ShaderSourceProc = void (*)(GLuint, GLsizei, const GLchar* const*, const GLint*);
using CompileShaderProc = void (*)(GLuint);
using GetShaderivProc = void (*)(GLuint, GLenum, GLint*);
using GetShaderInfoLogProc = void (*)(GLuint, GLsizei, GLsizei*, GLchar*);
using DeleteShaderProc = void (*)(GLuint);
using CreateProgramProc = GLuint (*)(void);
using AttachShaderProc = void (*)(GLuint, GLuint);
using LinkProgramProc = void (*)(GLuint);
using GetProgramivProc = void (*)(GLuint, GLenum, GLint*);
using GetProgramInfoLogProc = void (*)(GLuint, GLsizei, GLsizei*, GLchar*);
using UseProgramProc = void (*)(GLuint);
using DeleteProgramProc = void (*)(GLuint);
using GetUniformLocationProc = GLint (*)(GLuint, const GLchar*);
using UniformMatrix4fvProc = void (*)(GLint, GLsizei, GLboolean, const GLfloat*);
using Uniform3fProc = void (*)(GLint, GLfloat, GLfloat, GLfloat);
using Uniform1fProc = void (*)(GLint, GLfloat);
using Uniform1iProc = void (*)(GLint, GLint);

using GenVertexArraysProc = void (*)(GLsizei, GLuint*);
using BindVertexArrayProc = void (*)(GLuint);
using DeleteVertexArraysProc = void (*)(GLsizei, const GLuint*);
using GenBuffersProc = void (*)(GLsizei, GLuint*);
using BindBufferProc = void (*)(GLenum, GLuint);
using BufferDataProc = void (*)(GLenum, GLsizeiptr, const void*, GLenum);
using DeleteBuffersProc = void (*)(GLsizei, const GLuint*);
using EnableVertexAttribArrayProc = void (*)(GLuint);
using VertexAttribPointerProc = void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
using VertexAttribDivisorProc = void (*)(GLuint, GLuint);
using DrawElementsProc = void (*)(GLenum, GLsizei, GLenum, const void*);
using DrawElementsInstancedProc = void (*)(GLenum, GLsizei, GLenum, const void*, GLsizei);

using GenTexturesProc = void (*)(GLsizei, GLuint*);
using BindTextureProc = void (*)(GLenum, GLuint);
using TexImage2DProc = void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*);
using TexParameteriProc = void (*)(GLenum, GLenum, GLint);
using GenerateMipmapProc = void (*)(GLenum);
using ActiveTextureProc = void (*)(GLenum);
using DeleteTexturesProc = void (*)(GLsizei, const GLuint*);

extern CreateShaderProc createShader;
extern ShaderSourceProc shaderSource;
extern CompileShaderProc compileShader;
extern GetShaderivProc getShaderiv;
extern GetShaderInfoLogProc getShaderInfoLog;
extern DeleteShaderProc deleteShader;
extern CreateProgramProc createProgram;
extern AttachShaderProc attachShader;
extern LinkProgramProc linkProgram;
extern GetProgramivProc getProgramiv;
extern GetProgramInfoLogProc getProgramInfoLog;
extern UseProgramProc useProgram;
extern DeleteProgramProc deleteProgram;
extern GetUniformLocationProc getUniformLocation;
extern UniformMatrix4fvProc uniformMatrix4fv;
extern Uniform3fProc uniform3f;
extern Uniform1fProc uniform1f;
extern Uniform1iProc uniform1i;

extern GenVertexArraysProc genVertexArrays;
extern BindVertexArrayProc bindVertexArray;
extern DeleteVertexArraysProc deleteVertexArrays;
extern GenBuffersProc genBuffers;
extern BindBufferProc bindBuffer;
extern BufferDataProc bufferData;
extern DeleteBuffersProc deleteBuffers;
extern EnableVertexAttribArrayProc enableVertexAttribArray;
extern VertexAttribPointerProc vertexAttribPointer;
extern VertexAttribDivisorProc vertexAttribDivisor;
extern DrawElementsProc drawElements;
extern DrawElementsInstancedProc drawElementsInstanced;

extern GenTexturesProc genTextures;
extern BindTextureProc bindTexture;
extern TexImage2DProc texImage2D;
extern TexParameteriProc texParameteri;
extern GenerateMipmapProc generateMipmap;
extern ActiveTextureProc activeTexture;
extern DeleteTexturesProc deleteTextures;

bool load();

} // namespace plunger::gl