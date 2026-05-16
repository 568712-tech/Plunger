#include "renderer/OpenGLFunctions.h"

#include <SFML/Window/Context.hpp>

#include <stdexcept>

namespace plunger::gl {

CreateShaderProc createShader = nullptr;
ShaderSourceProc shaderSource = nullptr;
CompileShaderProc compileShader = nullptr;
GetShaderivProc getShaderiv = nullptr;
GetShaderInfoLogProc getShaderInfoLog = nullptr;
DeleteShaderProc deleteShader = nullptr;
CreateProgramProc createProgram = nullptr;
AttachShaderProc attachShader = nullptr;
LinkProgramProc linkProgram = nullptr;
GetProgramivProc getProgramiv = nullptr;
GetProgramInfoLogProc getProgramInfoLog = nullptr;
UseProgramProc useProgram = nullptr;
DeleteProgramProc deleteProgram = nullptr;
GetUniformLocationProc getUniformLocation = nullptr;
UniformMatrix4fvProc uniformMatrix4fv = nullptr;
Uniform3fProc uniform3f = nullptr;
Uniform1fProc uniform1f = nullptr;
Uniform1iProc uniform1i = nullptr;

GenVertexArraysProc genVertexArrays = nullptr;
BindVertexArrayProc bindVertexArray = nullptr;
DeleteVertexArraysProc deleteVertexArrays = nullptr;
GenBuffersProc genBuffers = nullptr;
BindBufferProc bindBuffer = nullptr;
BufferDataProc bufferData = nullptr;
DeleteBuffersProc deleteBuffers = nullptr;
EnableVertexAttribArrayProc enableVertexAttribArray = nullptr;
VertexAttribPointerProc vertexAttribPointer = nullptr;
DrawElementsProc drawElements = nullptr;

GenTexturesProc genTextures = nullptr;
BindTextureProc bindTexture = nullptr;
TexImage2DProc texImage2D = nullptr;
TexParameteriProc texParameteri = nullptr;
GenerateMipmapProc generateMipmap = nullptr;
ActiveTextureProc activeTexture = nullptr;
DeleteTexturesProc deleteTextures = nullptr;

template <typename T>
T loadFunction(const char* name)
{
    return reinterpret_cast<T>(sf::Context::getFunction(name));
}

bool load()
{
    createShader = loadFunction<CreateShaderProc>("glCreateShader");
    shaderSource = loadFunction<ShaderSourceProc>("glShaderSource");
    compileShader = loadFunction<CompileShaderProc>("glCompileShader");
    getShaderiv = loadFunction<GetShaderivProc>("glGetShaderiv");
    getShaderInfoLog = loadFunction<GetShaderInfoLogProc>("glGetShaderInfoLog");
    deleteShader = loadFunction<DeleteShaderProc>("glDeleteShader");

    createProgram = loadFunction<CreateProgramProc>("glCreateProgram");
    attachShader = loadFunction<AttachShaderProc>("glAttachShader");
    linkProgram = loadFunction<LinkProgramProc>("glLinkProgram");
    getProgramiv = loadFunction<GetProgramivProc>("glGetProgramiv");
    getProgramInfoLog = loadFunction<GetProgramInfoLogProc>("glGetProgramInfoLog");
    useProgram = loadFunction<UseProgramProc>("glUseProgram");
    deleteProgram = loadFunction<DeleteProgramProc>("glDeleteProgram");
    getUniformLocation = loadFunction<GetUniformLocationProc>("glGetUniformLocation");
    uniformMatrix4fv = loadFunction<UniformMatrix4fvProc>("glUniformMatrix4fv");
    uniform3f = loadFunction<Uniform3fProc>("glUniform3f");
    uniform1f = loadFunction<Uniform1fProc>("glUniform1f");
    uniform1i = loadFunction<Uniform1iProc>("glUniform1i");

    genVertexArrays = loadFunction<GenVertexArraysProc>("glGenVertexArrays");
    bindVertexArray = loadFunction<BindVertexArrayProc>("glBindVertexArray");
    deleteVertexArrays = loadFunction<DeleteVertexArraysProc>("glDeleteVertexArrays");
    genBuffers = loadFunction<GenBuffersProc>("glGenBuffers");
    bindBuffer = loadFunction<BindBufferProc>("glBindBuffer");
    bufferData = loadFunction<BufferDataProc>("glBufferData");
    deleteBuffers = loadFunction<DeleteBuffersProc>("glDeleteBuffers");
    enableVertexAttribArray = loadFunction<EnableVertexAttribArrayProc>("glEnableVertexAttribArray");
    vertexAttribPointer = loadFunction<VertexAttribPointerProc>("glVertexAttribPointer");
    drawElements = loadFunction<DrawElementsProc>("glDrawElements");

    genTextures = loadFunction<GenTexturesProc>("glGenTextures");
    bindTexture = loadFunction<BindTextureProc>("glBindTexture");
    texImage2D = loadFunction<TexImage2DProc>("glTexImage2D");
    texParameteri = loadFunction<TexParameteriProc>("glTexParameteri");
    generateMipmap = loadFunction<GenerateMipmapProc>("glGenerateMipmap");
    activeTexture = loadFunction<ActiveTextureProc>("glActiveTexture");
    deleteTextures = loadFunction<DeleteTexturesProc>("glDeleteTextures");

    return createShader && shaderSource && compileShader && getShaderiv && getShaderInfoLog && deleteShader &&
           createProgram && attachShader && linkProgram && getProgramiv && getProgramInfoLog && useProgram &&
           deleteProgram && getUniformLocation && uniformMatrix4fv && uniform3f && uniform1f && uniform1i && genVertexArrays &&
           bindVertexArray && deleteVertexArrays && genBuffers && bindBuffer && bufferData && deleteBuffers &&
           enableVertexAttribArray && vertexAttribPointer && drawElements && genTextures && bindTexture && texImage2D &&
           texParameteri && generateMipmap && activeTexture && deleteTextures;
}

} // namespace plunger::gl