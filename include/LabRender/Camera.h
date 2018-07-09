
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include <LabRender/Export.h>
#include <LabMath/LabMath.h>

namespace lab {

    class Mount 
    {
        m44f _viewTransform;

    public:
        Mount()
        : _viewTransform(m44f_identity) {}

        void        setViewTransform(m44f const& t) { _viewTransform = t; }
        const m44f& viewTransform() const           { return _viewTransform; }
        m44f        rotationTransform() const       { m44f j = _viewTransform; j[3] = {0,0,0,1}; return j; }

        v3f right()   const { return vector_normalize(V3F(_viewTransform[0].x,
                                                          _viewTransform[1].x,
                                                          _viewTransform[2].x)); }
        v3f up()      const { return vector_normalize(V3F(_viewTransform[0].y,
                                                          _viewTransform[1].y,
                                                          _viewTransform[2].y)); }
        v3f forward() const { return vector_normalize(V3F(_viewTransform[0].z,
                                                          _viewTransform[1].z,
                                                          _viewTransform[2].z)); }

        void lookat(v3f eye, v3f target, v3f up) { _viewTransform = lab::make_lookat_transform(eye, target, up); }
    };

    struct Sensor
	{
        float handedness = -1; // left handed
        v2f   aperture = { 35.f, 24.5f };
        v2f   enlarge =  { 1, 1 };
        v2f   shift =    { 0, 0 };
        v3f   lift =     { 0, 0, 0 };
        v3f   gain =     { 1, 1, 1 };
        v2f   knee =     { 1, 1 };
        v3f   gamma =    { 2.2f, 2.2f, 2.2f };
    };

    struct Optics 
    {
        MM    focalLength = 50.f;
        float zfar =        1e5f;
        float znear =       0.1f;
    };

    LR_API m44f    perspective(const Sensor& sensor, const Optics& optics);
    LR_API m44f    perspective(const Sensor& sensor, const Optics& optics, float aspect);
    inline Radians verticalFOV(const Sensor& sensor, const Optics& optics)
    {
        return 2.f * atanf(sensor.aperture.y / (2.f * optics.focalLength));
    }

    class Camera 
    {
    public:
        Mount  mount;
        Sensor sensor;
        Optics optics;

  		v3f position   { 0, 0, 0 };
		v3f worldUp    { 0, 1, 0 };
		v3f focusPoint { 0, 0, -10 };

        Camera()
        {
            updateViewTransform();
        }

        // Creates a matrix suitable for an OpenGL style MVP matrix
        // invert the view transform for graphics engines that pre-multiply
        //
        void updateViewTransform() 
        {
            mount.lookat(position, focusPoint, worldUp);
        }

        LR_API void frame(std::pair<v3f, v3f> bounds);
        LR_API void autoSetClippingPlanes(std::pair<v3f, v3f> bounds);
    };

    enum class CameraRigMode 
    {
        Dolly, Crane, TurnTableOrbit, Fly
    };

    // delta is the 2d motion of a mouse or gesture in the screen plane,
    // typically computed as scale * (currMousePos - prevMousePos);
    //
    LR_API void cameraRig_interact(Camera& camera, CameraRigMode mode, v2f delta);


} // Lab
