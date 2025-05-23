//
//  Renderer.h
//  LabRender
//
//

#pragma once

#include "LabRender/LabRender.h"
#include "LabRender/Texture.h"
#include "LabRender/ViewMatrices.h"
#include <LabCmd/Queue.h>

#include <atomic>
#include <functional>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

namespace lab { namespace Render {

class DrawList;

/**
    To render a frame, create a RenderLock.
*/

class Renderer 
{
protected:
    friend class RenderLock;
    friend class RenderContext;
    std::mutex  _renderLock;
    std::string _renderLockerId;

    Lab::mpmc_queue_blocking<std::function<void(void)>> _jobs;

public:
    class RenderLock;
    class RenderContext;

    Renderer() = default;
    virtual ~Renderer() = default;

    virtual std::shared_ptr<Render::Texture> texture(const std::string & name) = 0;
    virtual void render(RenderLock & rl, v2i fbSize, DrawList &) = 0;

    void enqueCommand(std::function<void(void)> && c) 
	{
        _jobs.emplace_back(std::move(c));
    }

    /**
        RenderLock is an RAII object that prevents re-entrancy during a render.
        */

    class RenderLock 
	{
		std::atomic<bool> _renderInProgress;
        Renderer* _dr = nullptr;

	public:
		class RenderContext
		{
		public:
			ViewMatrices viewMatrices;
			DrawList* drawList = nullptr;
			int activeTextureUnit = 0;
			v2i framebufferSize = { 0,0 };
			v2f mousePosition = { 0,0 };
			int32_t rootFramebuffer = 0;
			double renderTime = 0;
			std::unordered_map<std::string, std::shared_ptr<Render::Texture>> boundTextures;
		};

		RenderContext context;
	

		RenderLock(Renderer* dr, double renderTime, v2f mousePosition)
        : _renderInProgress(false)
        {
            if (dr && dr->_renderLock.try_lock()) {
                _dr = dr;
                context.mousePosition = mousePosition;
                context.renderTime = renderTime;

                // run any queued commands
                std::function<void(void)> run;
                do
                {
                    auto run = dr->_jobs.pop_front();
                    if (!run.first)
                        break;

                    run.second();
                }
                while (true);
            }
        }

        ~RenderLock()
        {
            // todo - check that renderStarted is zero
            if (_dr)
                _dr->_renderLock.unlock();
        }

        bool valid() const { return !!_dr; }

        bool renderInProgress() const    { return _renderInProgress.load(); }
        void setRenderInProgress(bool p) { _renderInProgress = p; }

        bool hasTexture(const std::string & name) const 
		{
			return context.boundTextures.find(name) != context.boundTextures.end();
        }

        void bindTexture(const std::string & name) 
		{
			auto i = context.boundTextures.find(name);
			if (i != context.boundTextures.end())
			{
				i->second->bind();
				return;
			}

            std::shared_ptr<Render::Texture> texture = _dr->texture(name);
			if (!texture)
				return;

			context.boundTextures[name] = texture;
            texture->bind();
        }
    };

};


}} // lab::Render
