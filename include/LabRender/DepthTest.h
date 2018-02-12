
#pragma once

#include <LabRender/LabRender.h>
#include <string>

namespace lab
{
    enum class DepthTest : unsigned int 
    {
        less, lequal, never, equal, greater, notequal, gequal, always
    };

    LR_API DepthTest stringToDepthTest(const std::string & s);

}
