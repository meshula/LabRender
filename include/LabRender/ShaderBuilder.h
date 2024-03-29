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
            std::string vtx_src,  fgmt_src,  fgmt_post_src;
            std::string vtx_path, fgmt_path, fgmt_post_path;

            std::vector<Uniform> uniforms;
            std::vector<Uniform> samplers;
            std::vector<std::pair<std::string, SemanticType>> attributes;
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
        void setAttributes(const ModelPart& mesh); /// @TODO deprecate

        void setAttributes(const ShaderSpec&);  /// @TODO ModelPart should call this explicitly
        void setUniforms(const ShaderSpec&);
        void setVaryings(const ShaderSpec&);
        void setSamplers(const ShaderSpec&);

        void setUniforms(Semantic const*const semantics, size_t count);
        void setVaryings(Semantic const*const semantics, size_t count);
        void setSamplers(Semantic const*const semantics, size_t count);

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
        std::map<std::string, Semantic*> attributes;
        std::set<Semantic*> varyings;
        std::set<Semantic*> outputs;
    };

}} // lab::Render
