//
//  Shader.cpp
//  LabRender
//
//  Created by Nick Porcino on 5/26/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#include "LabRender/Shader.h"
#include "LabRender/DrawList.h"
#include "gl4.h"

namespace lab { namespace Render {
    
Shader::~Shader() 
{
	if (id)
	    glDeleteProgram(id);

    for (size_t i = 0; i < stages.size(); i++)
        glDeleteShader(stages[i]);
}
    
namespace {
    GLint programTypeToGL[] = {
        GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
        GL_GEOMETRY_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER
    };
}
    
Shader& Shader::shader(const std::string& name, ProgramType type, bool autoPreamble, char const*const source) 
{
    // Compile shader
    int itype = static_cast< typename std::underlying_type<ProgramType>::type >(type);
    unsigned int shader = glCreateShader(programTypeToGL[itype]);
        
    std::string src;
    if (autoPreamble) {
        src = "//"  + name + "\n\n\
#version 410\n\
#extension GL_ARB_explicit_attrib_location : enable\n\
#define texture2D texture\n";
    }
    src += source;
    const char* pStr = src.c_str();
        
    glShaderSource(shader, 1, &pStr, NULL);
    glCompileShader(shader);
    stages.push_back(shader);
        
    // Check for errors
    char buffer[512];
    int length;
    glGetShaderInfoLog(shader, sizeof(buffer), &length, buffer);
    if (length)
        handleGLError(errorPolicy, GL_INVALID_VALUE, buffer, source);
        
    // Allow chaining
    return *this;
}
    
void Shader::link() 
{
    // Create and link program
    if (!id) id = glCreateProgram();
    for (size_t i = 0; i < stages.size(); i++) {
        glAttachShader(id, stages[i]);
    }
    glLinkProgram(id);
        
    // Check for errors
    char buffer[512];
    int length;
    glGetProgramInfoLog(id, sizeof(buffer), &length, buffer);
        
    if (length != 0)
        handleGLError(errorPolicy, GL_INVALID_OPERATION, buffer);
        
    GLenum glErr = glGetError();
    if (glErr)
        handleGLError(errorPolicy, glErr, buffer);
}

void Shader::bind(Renderer::RenderLock& rl) const 
{
	checkError(ErrorPolicy::onErrorThrow,
		TestConditions::exhaustive, "Shader::bind");

	glUseProgram(id);

    checkError(ErrorPolicy::onErrorThrow,
                TestConditions::exhaustive, "Shader::bind useProgram");

    int activeTextureUnit = rl.context.activeTextureUnit;
    for (auto t : sampledTextures) 
	{
        if (rl.hasTexture(t.texture)) {
            glActiveTexture(GL_TEXTURE0 + activeTextureUnit);
            rl.bindTexture(t.texture);
            uniformInt(t.name.c_str(), activeTextureUnit);
            ++activeTextureUnit;
        }
    }
    rl.context.activeTextureUnit = activeTextureUnit;
        
    for (auto a : automatics) 
    {
        if (a.automatic == AutomaticUniform::frameBufferResolution) 
        {
            uniform(a.name.c_str(), V2F(rl.context.framebufferSize.x, rl.context.framebufferSize.y));
        }
        else if (a.automatic == AutomaticUniform::skyMatrix) 
        {
            m44f projection = rl.context.drawList->proj;
            m44f skyMatrix = matrix_invert(matrix_multiply(projection, rl.context.drawList->modl));
            uniform(a.name.c_str(), skyMatrix);
        }
        else if (a.automatic == AutomaticUniform::renderTime) 
        {
            uniformFloat(a.name.c_str(), (float) rl.context.renderTime);
        }
        else if (a.automatic == AutomaticUniform::mousePosition) 
        {
            uniform(a.name.c_str(), rl.context.mousePosition);
        }
    }

    checkError(ErrorPolicy::onErrorThrow,
                TestConditions::exhaustive, "Shader::bind set uniforms");
}
    
void Shader::unbind() const { glUseProgram(0); }


unsigned int Shader::attribute(const char *name) const { return glGetAttribLocation(id, name); }
unsigned int Shader::uniform(const char *name)   const { return glGetUniformLocation(id, name); }
    
void Shader::uniformInt(const char *name, int i)     const { glUniform1i(uniform(name), i); }
void Shader::uniformFloat(const char *name, float f) const { glUniform1f(uniform(name), f); }
void Shader::uniform(const char *name, const v2f &v) const { glUniform2fv(uniform(name), 1, (float*)&v); }
void Shader::uniform(const char *name, const v3f &v) const { glUniform3fv(uniform(name), 1, (float*)&v); }
void Shader::uniform(const char *name, const v4f &v) const { glUniform4fv(uniform(name), 1, (float*)&v); }
    
void Shader::uniform(const char *name, const m44f &m, bool transpose) const {
    glUniformMatrix4fv(uniform(name), 1, transpose, (float*)&m); }


}} // lab::Render
