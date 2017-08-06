//
//  TextureLoader.h
//  LabApp
//
//  Created by Nick Porcino on 2014 03/1.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

namespace lab {

    /// @TODO these should be a texture provider.
    /// ie: class CGTextureProvider : public TextureProvider

    class Texture;
    std::unique_ptr<Texture> loadTexture(char const*const url, bool asCube = false);
    std::unique_ptr<Texture> loadCubeTexture(std::string url[6]);

} // Lab
