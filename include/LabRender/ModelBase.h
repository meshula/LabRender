//
//  ModelBase.h
//  LabRender
//
//  Created by Nick Porcino on 8/7/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#include "LabRender/Renderer.h"
#include <memory>

namespace lab { namespace Render {
    
    struct FrameBuffer;
    class Material;
    
    class ModelBase {
    public:
		LR_API virtual ~ModelBase() { }
        virtual void update(double time) = 0;
        virtual void draw() = 0;
        virtual void draw(
            const FrameBuffer& fbo, const std::vector<std::string>& output_attachments, 
            Renderer::RenderLock &) = 0;
        virtual Bounds localBounds() const = 0;
        
        std::shared_ptr<Material> material;
    };

}}
