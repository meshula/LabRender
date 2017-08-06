//
//  ModelBase.h
//  LabRender
//
//  Created by Nick Porcino on 8/7/15.
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#include "LabRender/Transform.h"
#include "LabRender/Renderer.h"
#include <memory>

namespace lab {
    
    struct FrameBuffer;
    class Material;
    
    class ModelBase {
    public:
		LR_API virtual ~ModelBase() { }
        virtual void update(double time) = 0;
        virtual void draw() = 0;
        virtual void draw(FrameBuffer & fbo, Renderer::RenderLock &) = 0;
        virtual Bounds localBounds() const = 0;
        
        Transform transform;
        
        std::shared_ptr<Material> material;
    };

}
