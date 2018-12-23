
#pragma once

#include <LabRender/LabRender.h>
#include <LabRenderTypes/DepthTest.h>
#include <string>

namespace lab
{

    LR_API Render::DepthTest stringToDepthTest(const std::string & s);

}
