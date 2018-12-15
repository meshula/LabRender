//
//  LabShader.h
//  LabApp
//
//  Created by Nick Porcino on 2014 01/28.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include "LabRender/Model.h"
#include "LabRender/SemanticType.h"
#include <vector>
#include <set>
#include <string>

namespace lab { namespace Render {

    class ShaderBuilder 
	{
    public:

        class ShaderSpec 
		{
        public:
            std::string name;
            std::string vertexShaderPath;
            std::string fragmentShaderPath;
            std::string fragmentShaderPostamblePath;

            std::vector<Uniform> uniforms;
            std::vector<Uniform> samplers;
            std::vector<std::pair<std::string, SemanticType>> varyings;
        };

        class Cache 
		{
        public:
            Cache();
            ~Cache();
            bool hasShader(const std::string&) const;
            void add(const std::string&, std::shared_ptr<Shader>);
            std::shared_ptr<Shader> shader(const std::string&) const;

        private:
            class Detail;
            std::unique_ptr<Detail> _detail;
        };
        static Cache* cache();

        ~ShaderBuilder();

        void clear();
        void setFrameBufferOutputs(const FrameBuffer& fbo, const std::vector<std::string>& output_attachments);
        void setAttributes(const ModelPart& mesh);

        void setUniforms(const ShaderSpec&);
        void setVaryings(const ShaderSpec&);
        void setSamplers(const ShaderSpec&);

        void setUniforms(Semantic const*const semantics, int count);
        void setVaryings(Semantic const*const semantics, int count);
        void setSamplers(Semantic const*const semantics, int count);

        std::string generateVertexShader(const char* body);
        std::string generateFragmentShader(const char* body);

        std::shared_ptr<Shader> makeShader(const std::string& name,
                                           const char* vtxCode, const char* fgmtCode,
                                           const VAO& vao,
                                           bool printShader = false);
        std::shared_ptr<Shader> makeShader(const ShaderSpec&,
                                           const VAO& vao,
                                           bool printShader = false);

        std::set<Semantic*> uniforms;
        std::set<Semantic*> samplers;
        std::set<Semantic*> attributes;
        std::set<Semantic*> varyings;
        std::set<Semantic*> outputs;
    };

}} // lab::Render
