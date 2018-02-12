#pragma once

#include <LabRender/LabRender.h>
#include <LabRender/SemanticType.h>
#include <string>

namespace lab
{


    enum class AutomaticUniform {
        none,
        frameBufferResolution,
        skyMatrix,
        renderTime,
        mousePosition,
    };

    LR_API AutomaticUniform stringToAutomaticUniform(const std::string & s);
    
    class Uniform {
    public:
        Uniform(std::string name,
            SemanticType type,
            AutomaticUniform automatic,
            std::string texture)
            : name(name), type(type), automatic(automatic), texture(texture) {}

        Uniform(const Uniform & rhs)
            : name(rhs.name), type(rhs.type), automatic(rhs.automatic), texture(rhs.texture) {}

        Uniform & operator=(const Uniform & rhs) {
            name = rhs.name; type = rhs.type; automatic = rhs.automatic; texture = rhs.texture;
            return *this;
        }

        std::string name;
        std::string texture;
        SemanticType type = SemanticType::unknown_st;
        AutomaticUniform automatic = AutomaticUniform::none;
    };

}