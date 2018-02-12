//
//  Renderer.h
//  LabRender
//
//

#pragma once

#include "LabRender/LabRender.h"
#include "LabRender/ConcurrentQueue.h"
#include "LabRender/Texture.h"
#include "LabRender/ViewMatrices.h"

#include <atomic>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include <string>

namespace lab {

    class DrawList;
    struct Texture;

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

        concurrent_queue<std::function<void(void)>> _jobs;

    public:
        class RenderLock;
        class RenderContext;

        virtual ~Renderer() {}

        virtual std::shared_ptr<Texture> texture(const std::string & name) = 0;
        virtual void render(RenderLock & rl, v2i fbSize, DrawList &) = 0;

        void enqueCommand(std::function<void(void)> c) 
		{
            _jobs.push(c);
        }



        /**
         RenderLock is an RAII object that prevents re-entrancy during a render.
         */

        class RenderLock 
		{
			std::atomic<bool> _renderInProgress;
			Renderer* _dr;

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
				std::unordered_map<std::string, std::shared_ptr<Texture>> boundTextures;
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
                    while (dr->_jobs.try_pop(run))
                        run();
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

                std::shared_ptr<Texture> texture = _dr->texture(name);
				if (!texture)
					return;

				context.boundTextures[name] = texture;
                texture->bind();
            }
        };

    };


}
