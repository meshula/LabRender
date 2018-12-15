
#ifndef LABRENDER_TEXTURETYPE_H
#define LABRENDER_TEXTURETYPE_H

#include <LabRender/LabRender.h>
#include <LabRenderTypes/Texture.h>
#include <string>

namespace lab { namespace Render {
    LR_API Render::TextureType stringToTextureType(const std::string & s);
}}

#endif
