//
//  Material.h
//  LabApp
//
//  Created by Nick Porcino on 2014 03/5.
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once
#include "InOut.h"
#include "Texture.h"

namespace lab { namespace Render {

    class Material {
    public:
        Material();
        ~Material();

        // properties ~ setting these replaces the default inouts.
        void setPropertyInlet(const std::string & name, std::shared_ptr<InOut> & inlet);
        void setPropertyInlet(char const*const name, std::shared_ptr<InOut> & inlet);

        std::shared_ptr<InOut> propertyInlet(const std::string & name) const;
        std::shared_ptr<InOut> propertyInlet(char const*const name) const;

    private:
        class Detail;
        Detail* _detail;
    };

    class ShaderMaterial : Material {
    public:
        virtual ~ShaderMaterial() {}

        // predefined names
        static char const*const baseColorName()          { return "baseColor"; }
        static char const*const depthWriteName()         { return "depthWrite"; }
        static char const*const depthRangeName()         { return "depthRange"; }
        static char const*const drawOrderName()          { return "drawOrder"; }
        static char const*const depthFuncName()          { return "depthFunc"; }
        static char const*const vertexShaderFileName()   { return "vertexShaderFile"; }
        static char const*const fragmentShaderFileName() { return "fragmentShaderFile"; }

    };

    class DisneyMaterial : ShaderMaterial {
    public:
        virtual ~DisneyMaterial() {}

        // predefined names
        static char const*const metalnessName()     { return "metalness"; }
        static char const*const roughnessName()     { return "rougness"; }
        static char const*const glossNameName()     { return "gloss"; }
        static char const*const glossTintName()     { return "glossTint"; }
        static char const*const specularName()      { return "specular"; }
        static char const*const specularTintName()  { return "specularTint"; }
        static char const*const subsurfaceName()    { return "subsurface"; }
        static char const*const sheenName()         { return "sheen"; }
        static char const*const sheenTintName()     { return "sheenTint"; }
        static char const*const clearCoatName()     { return "clearCoat"; }
        static char const*const clearCoatTintName() { return "clearCoatTint"; }
    };

}} // lab::Render
