
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


#pragma once

#include "LSystem.h"
#include "LabRender/Model.h"

namespace LabRender {
    
    class LSystemMesh : public ModelPart {
    public:
        LSystemMesh();
        virtual ~LSystemMesh();
        
        VAO* vertData;
    };
    
    
    void renderLSystem(LSystemMesh* mesh, float depth_comparison, const std::vector<LSystem::Shape>&,
                       const glm::mat4x4& invCamera);
    
}