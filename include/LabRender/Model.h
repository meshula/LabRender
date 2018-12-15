//
//  Model.h
//  LabApp
//
//  Created by Nick Porcino on 2013 12/29.
//  Copyright (c) 2013 Nick Porcino. All rights reserved.
//

#pragma once

#include <LabRender/LabRender.h>
#include "LabRender/FrameBuffer.h"
#include "LabRender/ModelBase.h"
#include "LabRender/Shader.h"
#include "LabRender/Vertex.h"
#include "LabRender/ViewMatrices.h"
#include <iostream>

namespace lab { namespace Render {

    struct FrameBuffer;

    class ModelPart : public ModelBase 
	{
    public:
        enum class ShaderType {
            meshShader,
            skyShader, customShader };

		LR_API ModelPart() : _shaderType(ShaderType::meshShader) {}
		LR_API virtual ~ModelPart() {}

		LR_API void setVAO(std::unique_ptr<VAO>, Bounds localBounds);

		LR_API void setShaderType(ShaderType st) { _shaderType = st; }

		LR_API virtual void update(double time) override {}

		LR_API virtual void draw() override 
		{
            if (_verts && _verts->uploadVerts())
                _verts->draw(); 
		}

		LR_API virtual void draw(
            const FrameBuffer& fbo, const std::vector<std::string>& output_attachments,
            Renderer::RenderLock &) override;

		LR_API VAO * verts() const { return _verts.get(); }

		LR_API void setShader(std::shared_ptr<Shader> shader) { _shader = shader; }
		LR_API std::shared_ptr<Shader> shader() const { return _shader; }

		LR_API static char const*const defaultShaderSourceId() { return "default"; }

        // passing in nullptr for vshSrc or fshSrc will cause the corresponding shader to be auto generated
        LR_API static std::shared_ptr<Shader> makeShader(
            const FrameBuffer& fbo, const std::vector<std::string>& output_attachments, 
            ModelPart & mesh, ShaderType shaderType,
            char const*const vshSrc = 0, char const*const fshSrc = 0);

		LR_API virtual Bounds localBounds() const override {
            return _localBounds;
        }

    protected:
        ShaderType              _shaderType;
        std::shared_ptr<Shader> _shader;
        std::unique_ptr<VAO>    _verts;
        Bounds                  _localBounds;
    };

    class Model 
    {
    public:
		LR_API ~Model() {}

		LR_API void addPart(std::shared_ptr<ModelBase> p) { _parts.push_back(p); }
		LR_API Bounds localBounds() const;
        LR_API void addToDrawList(DrawList&);
        LR_API std::vector<std::shared_ptr<ModelBase>> parts() const { return _parts;}

    protected:
        std::vector<std::shared_ptr<ModelBase>> _parts;
    };

}} // lab::Render
