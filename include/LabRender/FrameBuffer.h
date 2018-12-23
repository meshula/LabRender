//
//  FrameBuffer.h
//  LabRender
//
//  Created by Nick Porcino on 5/21/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once
#include "LabRender/ErrorPolicy.h"
#include "LabRender/SemanticType.h"
#include "LabRender/Texture.h"
#include <LabMath/LabMath.h>
#include <map>
#include <vector>
#include <string>

namespace lab { namespace Render {

    // A framebuffer object that takes color attachments. Draw calls between
    // bind() and unbind() are drawn to the attached textures.
    //
    // Usage:
    //
    //     FrameBuffer fbo;
    //
    //     fbo.attachColor(texture).checkFbo();
    //     fbo.bind();
    //     // draw stuff
    //     fbo.unbind();
    //
    struct FrameBuffer 
	{
		unsigned int id = 0;
        unsigned int renderbuffer = 0;
        bool autoDepth = true;
        bool resizeViewport = true;
        int newViewport[4], oldViewport[4];
        int renderbufferWidth = 0, renderbufferHeight = 0;
        ErrorPolicy errorPolicy;
        std::vector<std::string> baseNames;         // index is implicity the attachment number
        std::vector<std::string> drawBufferNames;
        std::vector<std::string> uniformNames;
        std::vector<Render::SemanticType> samplerType;
        std::vector<std::shared_ptr<Render::Texture>> textures;

        FrameBuffer(ErrorPolicy ep = ErrorPolicy::onErrorLogThrow, bool autoDepth = true, bool resizeViewport = true);
        ~FrameBuffer();

        // Draw calls between these will be drawn to attachments. If resizeViewport
        // is true this will automatically resize the viewport to the size of the
        // last attached texture.
        void bindForRead();
		void bindForWrite();
        void unbind();

        void bindForRead(const std::vector<std::string> & attachments);
		void bindForWrite(const std::vector<std::string> & attachments);

        class FrameBufferSpec 
        {
        public:
            class AttachmentSpec 
            {
            public:
                AttachmentSpec() {}

                AttachmentSpec(const std::string & b, const std::string & o, const std::string & u, Render::TextureType t)
                : base_name(b), output_name(o), uniform_name(u), type(t) {}

                AttachmentSpec(const AttachmentSpec & rh)
                : base_name(rh.base_name), output_name(rh.output_name), uniform_name(rh.uniform_name), type(rh.type) {}

                const AttachmentSpec & operator=(const AttachmentSpec & rh) {
                    base_name = rh.base_name; output_name = rh.output_name; uniform_name = rh.uniform_name; type = rh.type;
                    return *this;
                }

                std::string base_name;
                std::string output_name;
                std::string uniform_name;
                Render::TextureType type;
            };

            FrameBufferSpec() {}
            FrameBufferSpec(const FrameBufferSpec & rh) : attachments(rh.attachments), hasDepth(rh.hasDepth) {}

            std::vector<AttachmentSpec> attachments;
            bool hasDepth;
        };
        void createAttachments(const FrameBufferSpec &, int width, int height);

        // Draw to texture 2D in the indicated attachment location (or a 2D layer of
        // a 3D texture).
        // Uniform name is the name the attachment is to take during post processing
        FrameBuffer& attachColor(char const*const base_name, char const*const drawbuffer_name, char const*const uniform_name,
                         const Render::Texture &texture, unsigned int attachment = 0, unsigned int layer = 0);

        // Stop drawing to the indicated color attachment
        FrameBuffer& detachColor(unsigned int attachment = 0);

        // Call after all attachColor() calls, validates attachments.
        FrameBuffer& checkFbo();
    };

    class FramebufferSet 
    {
    public:
        FramebufferSet();
        ~FramebufferSet() {}

        void addFbo(const std::string & name, const FrameBuffer::FrameBufferSpec &);
        std::shared_ptr<FrameBuffer> fbo(const std::string & named) const;

        bool setSize(int width, int height);

    private:
        int _width, _height;
        std::map<std::string, std::pair<FrameBuffer::FrameBufferSpec, std::shared_ptr<FrameBuffer>>> _fbos;
    };

}} // lab::Render
