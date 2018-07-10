//
//  Light.h
//  LabApp
//
//  Copyright (c) 2014 Nick Porcino. All rights reserved.
//

#pragma once

#include <LabMath/LabMath.h>

namespace lab {

    class Light 
	{
    public:
        virtual ~Light() {}
    };

    class SkyDomeLight : public Light {
    public:
        virtual ~SkyDomeLight() {}
    };

    class PointLight : public Light {
    public:
        virtual ~PointLight() {}
    };

    struct Illuminant {
        Illuminant(
            std::shared_ptr<Light> & light,
            lab::m44f transform)
            : light(light), transform(transform) {}
        Illuminant(const Illuminant & rhs) : light(rhs.light), transform(rhs.transform) {}
        std::shared_ptr<Light> light;
        lab::m44f transform;
    };

} // Lab
