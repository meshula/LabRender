//
//  PassRenderer.h
//  LabApp
//
//  Created by Nick Porcino on 2014 01/19.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include <LabRender/LabRender.h>
#include <LabRender/DepthTest.h>
#include <LabRender/DrawList.h>
#include <LabRender/FrameBuffer.h>
#include <LabRender/Model.h>
#include <LabRender/Renderer.h>
#include <LabRender/Shader.h>
#include <LabRender/ShaderBuilder.h>
#include <LabRender/Texture.h>

#include <string>

namespace lab { namespace Render {

    class PassRenderer : public Renderer
	{
    public:

        class Pass
		{
			std::string _name;
			int _passNumber;
			std::shared_ptr<Shader> _shader;

		public:

            Pass(const std::string& name, int passNumber);
            virtual ~Pass() {}

            int passNumber() const { return _passNumber; }

            void bindInputTextures(RenderLock &, const FramebufferSet &);

            virtual void run(RenderLock &, const FramebufferSet &);

            std::string name() const { return _name; }

            ShaderBuilder::ShaderSpec shaderSpec;

            std::string writeBuffer;
            std::vector<std::string> writeAttachments;
            std::vector<std::pair<std::string, std::vector<std::string>>> readAttachments;

            DepthTest depthTest = DepthTest::less;
            bool writeDepth = true;
            bool clearDepthBuffer = false;
            bool clearGbuffer = false;
            bool isQuadPass = false;
            bool drawOpaqueGeometry = false;

            std::shared_ptr<ModelPart> _fullScreenQuadMesh;

            void prepareFullScreenQuadAndShader(const FramebufferSet&);
        };

        LR_API PassRenderer();
        LR_API virtual ~PassRenderer();

        LR_API void configure(char const*const path);

        LR_API virtual std::shared_ptr<Render::Texture> texture(const std::string & name) override;

        LR_API std::shared_ptr<FrameBuffer> framebuffer(const std::string & name);

        LR_API std::shared_ptr<Pass> addPass(std::shared_ptr<Pass>);

        template <typename T>
        T* findPass(const std::string & name) const {
            auto pass = _findPass(name);
            return dynamic_cast<T*>(pass);
        }

        LR_API virtual void render(RenderLock & rl, v2i fbSize, DrawList &) override;

    private:
        Pass* _findPass(const std::string &) const;

        class Detail;
        Detail *_detail;
    };

}} // lab::Render
