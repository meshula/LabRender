//
//  Shader.h
//  LabRender
//
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#include <LabRender/ErrorPolicy.h>
#include <LabRender/Renderer.h>
#include <LabRender/Uniform.h>
#include <LabMath/LabMath.h>

namespace lab { namespace Render {

    /*
     Shader program compiler, linker, and binder
     */

    struct Shader
	{
        std::vector<Uniform> automatics;
        std::vector<Uniform> sampledTextures;

        uint32_t id = 0;
        ErrorPolicy errorPolicy;
        std::vector<unsigned int> stages;

        Shader(ErrorPolicy ep = ErrorPolicy::onErrorThrow) : id(), errorPolicy(ep) {}
        ~Shader();

        enum class ProgramType { Vertex = 0, Fragment, Geometry, TessControl, TessEval };

        Shader & shader(const std::string & name, ProgramType type, bool autoPreamble, char const*const source);

        void link();
        void bind(Renderer::RenderLock & rl) const;
        void unbind() const;

        unsigned int attribute(const char *name) const;
        unsigned int uniform(const char *name) const;

        void uniformInt(const char *name, int i) const;
        void uniformFloat(const char *name, float f) const;
        void uniform(const char *name, const v2f &v) const;
        void uniform(const char *name, const v3f &v) const;
        void uniform(const char *name, const v4f &v) const;

        void uniform(const char *name, const m44f &m, bool transpose = false) const;
    };

}}
