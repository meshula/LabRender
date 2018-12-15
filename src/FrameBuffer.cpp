//
//  FrameBuffer.cpp
//  LabRender
//
//  Copyright (c) 2013 Planet IX. All rights reserved.
//

#include "LabRender/FrameBuffer.h"
#include "gl4.h"
#include "LabRender/Texture.h"
#include <iostream>

using namespace std;

namespace lab {

    using namespace Render;

    FrameBuffer::FrameBuffer(ErrorPolicy ep, bool autoDepth, bool resizeViewport)
    : id()
    , renderbuffer()
    , autoDepth(autoDepth)
    , resizeViewport(resizeViewport)
    , newViewport()
    , oldViewport()
    , renderbufferWidth()
    , renderbufferHeight()
    , errorPolicy(ep)
    {}

    FrameBuffer::~FrameBuffer()
    {
        if (id)
             glDeleteFramebuffers(1, &id);

        if (renderbuffer)
            glDeleteRenderbuffers(1, &renderbuffer);
    }

    void FrameBuffer::createAttachments(const FrameBufferSpec& spec, int width, int height)
    {
        textures.clear();
        if (!spec.attachments.size()) {
            // special case; no attachments means the default frame buffer
            return;
        }

        textures.reserve(spec.attachments.size() + (spec.hasDepth ? 1 : 0));

        try {
            for (int i = 0; i < spec.attachments.size(); ++i) {
                textures.emplace_back(std::make_shared<Texture>());
                textures[i]->create(width, height, spec.attachments[i].type, GL_NEAREST, GL_CLAMP_TO_EDGE);
                attachColor(spec.attachments[i].base_name.c_str(),
                            spec.attachments[i].output_name.c_str(),
                            spec.attachments[i].uniform_name.c_str(),
                            *textures[i], i);
            }
            if (spec.hasDepth) {
                textures.emplace_back(std::make_shared<Texture>());
                textures[spec.attachments.size()]->createDepth(width, height);
				int i = int(textures.size() - 1);
				attachColor("_depth", "o__depthTexture", "u__depthTexture", *textures[i], i);
            }
            checkFbo();
        }
        catch (const std::exception & exc) {
            if (errorPolicy == ErrorPolicy::onErrorLogThrow) {
                std::cerr << exc.what();
                throw;
            }
            if (errorPolicy == ErrorPolicy::onErrorThrow) {
                throw;
            }
            if (errorPolicy == ErrorPolicy::onErrorLog) {
                std::cerr << exc.what();
            }
            else
                exit(1);
        }
    }

    void FrameBuffer::bindForRead() 
	{
		size_t c = textures.size();
		for (size_t i = 0; i < c; ++i)
			if (!!textures[i])
				textures[i]->bind((int) i);
    }

	void FrameBuffer::bindForWrite()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
		if (resizeViewport)
		{
			glGetIntegerv(GL_VIEWPORT, oldViewport);
			glViewport(newViewport[0], newViewport[1], newViewport[2], newViewport[3]);
		}
	}

    void FrameBuffer::bindForRead(const std::vector<std::string> & attachments) 
	{
        bindForRead();
#if 0
        vector<GLenum> currentDrawBuffers;
        for (auto a : attachments) {
            for (int i = 0; i < baseNames.size(); ++i) {
                if (a == baseNames[i]) {
                    currentDrawBuffers.push_back(drawBuffers[i]);
                    break;
                }
            }
        }
#endif
	}

	void FrameBuffer::bindForWrite(const std::vector<std::string> & attachments)
	{
		bindForWrite();
        std::array<GLenum, 32> currentDrawBuffers;
        GLsizei idx = 0;
		for (auto a : attachments) 
        {
			for (int i = 0; i < baseNames.size(); ++i) 
            {
				if (a == baseNames[i]) 
                {
					currentDrawBuffers[idx] = drawBuffers[i];
                    ++idx;
					break;
				}
			}
		}
		if (idx > 0)
			glDrawBuffers(idx, currentDrawBuffers.data());
	}

    void FrameBuffer::unbind() 
	{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (resizeViewport) {
            glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
        }
    }

    FrameBuffer& FrameBuffer::attachColor(char const*const base_name,
                                          char const*const output_name,
                                          char const*const uniform_name,
                                          const lab::Texture &texture,
                                          unsigned int attachment, unsigned int layer)
    {
        newViewport[2] = texture.width;
        newViewport[3] = texture.height;
        if (!id)
            glGenFramebuffers(1, &id);

		glBindFramebuffer(GL_FRAMEBUFFER, id);

        // Bind a 2D texture (using a 2D layer of a 3D texture)
        if (texture.depthTexture)
            glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture.id, 0);
        else if (texture.target == GL_TEXTURE_2D)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, texture.target, texture.id, 0);
        else
            glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment, texture.target, texture.id, 0, layer);

        // Need to call glDrawBuffers() for OpenGL to draw to multiple attachments
        if (!texture.depthTexture) 
		{
            if (attachment >= drawBuffers.size()) {
                drawBuffers.resize(attachment + 1, GL_NONE);
                drawBufferNames.resize(attachment + 1);
                baseNames.resize(attachment + 1);
                uniformNames.resize(attachment + 1);
                samplerType.resize(attachment + 1);
            }
            drawBufferNames[attachment] = std::string(output_name);
            baseNames[attachment] = std::string(base_name);
            uniformNames[attachment] = std::string(uniform_name);
            drawBuffers[attachment] = GL_COLOR_ATTACHMENT0 + attachment;
            samplerType[attachment] = glFormatToSemanticType(texture.format);

            //glDrawBuffers((GLsizei) drawBuffers.size(), drawBuffers.data());
        }
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return *this;
    }

    FrameBuffer& FrameBuffer::detachColor(unsigned int attachment)
    {
		glBindFramebuffer(GL_FRAMEBUFFER, id);

        // Update the draw buffers
        if (attachment < drawBuffers.size()) {
            drawBuffers[attachment] = GL_NONE;
            //glDrawBuffers((GLsizei) drawBuffers.size(), drawBuffers.data());
        }

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return *this;
    }

    FrameBuffer& FrameBuffer::checkFbo()
    {
		glBindFramebuffer(GL_FRAMEBUFFER, id);
		if (autoDepth)
		{
            if (!renderbuffer || renderbufferWidth != newViewport[2] || renderbufferHeight != newViewport[3]) 
			{
                renderbufferWidth = newViewport[2];
                renderbufferHeight = newViewport[3];
                if (!renderbuffer) 
					glGenRenderbuffers(1, &renderbuffer);
                glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, renderbufferWidth, renderbufferHeight);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
        }
        GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (result != GL_FRAMEBUFFER_COMPLETE)
            handleGLError(errorPolicy, result, "FrameBuffer::check()");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return *this;
    }




    FramebufferSet::FramebufferSet()
    : _width(0), _height(0)
    {}

    std::shared_ptr<FrameBuffer> FramebufferSet::fbo(const std::string & named) const
    {
        auto i = _fbos.find(named);
        if (i == _fbos.end())
            return nullptr;
        return i->second.second;
    }

    void FramebufferSet::addFbo(const std::string & name, const FrameBuffer::FrameBufferSpec & spec)
    {
        _fbos[name] = std::make_pair(spec, std::make_shared<FrameBuffer>());
    }

    bool FramebufferSet::setSize(int width, int height) 
	{
        if (_width == width && _height == height) {
            return true;
        }
        if (width == 0 && height == 0) {
            return true;
        }
        _width = width;
        _height = height;

        for (auto i : _fbos) {
            i.second.second->createAttachments(i.second.first, width, height);
        }

        return true;
    }


}
