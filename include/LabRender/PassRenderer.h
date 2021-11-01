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

#include <functional>
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
            std::shared_ptr<ModelPart> _fullScreenQuadMesh;

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

            // vector<fbo, texture>
            std::vector<std::pair<std::string, std::string>> readAttachments;

            DepthTest depthTest = DepthTest::less;
            bool active = true;
            bool writeDepth = true;
            bool clearDepthBuffer = false;
            bool clearGbuffer = false;
            bool isQuadPass = false;
            bool drawOpaqueGeometry = false;

            std::function<void()> renderPlug;

            void prepareFullScreenQuadAndShader(const FramebufferSet&);
        };

        LR_API PassRenderer();
        LR_API virtual ~PassRenderer();

        LR_API void configure(char const*const path);

        LR_API virtual std::shared_ptr<Render::Texture> texture(const std::string & name);

        LR_API std::shared_ptr<FrameBuffer> framebuffer(const std::string & name);

        LR_API std::shared_ptr<Pass> addPass(std::shared_ptr<Pass>);

        template <typename T>
        T* findPass(const std::string & name) const {
            auto pass = _findPass(name);
            return dynamic_cast<T*>(pass);
        }

        LR_API virtual void render(RenderLock & rl, v2i fbSize, DrawList &) override;

        LR_API std::function<void()> findPlug(char const* const name);
        LR_API void registerPlug(char const* const name, std::function<void()>);

    private:
        LR_API Pass* _findPass(const std::string &) const;

        class Detail;
        Detail *_detail;
    };

}} // lab::Render
