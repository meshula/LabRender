
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include <LabRender/LabRender.h>
#include "LabRender/Transform.h"
#include "LabRender/MathTypes.h"

#include <algorithm>
#include <memory>

namespace lab {

    class Sensor
	{
    public:
        Sensor()
        : _apertureHeight(24.5f)
        , _lift(V3F(0,0,0))
        , _gain(V3F(1,1,1))
        , _knee(V2F(1,1))
        , _gamma(V3F(2.2f,2.2f,2.2f))
        {}

        MM apertureHeight() const { return _apertureHeight; }

        MM _apertureHeight;
        v3f _lift;
        v3f _gain;
        v2f _knee;
        v3f _gamma;
    };

    class Optics {
    public:
        Optics()
        : _focalLength(50)
        , _znear(0.1f)
        , _zfar(1e5f)
        {
            update();
        }

        Radians verticalFOV() const { return _verticalFOV; }

        void setFocalLength(MM l) { _focalLength = l; update(); }

        void setZclip(float znear, float zfar) { _znear = znear; _zfar = zfar; }
        float znear() const { return _znear; }
        float zfar() const { return _zfar; }

        m44f perspective(float aspect) const {
            if (fabs(aspect) < std::numeric_limits<float>::epsilon())
                return m44f_identity;

            const float handedness = -1.f; // 1 for left hand coordinates
            float left = -1.f, right = 1.f, bottom = -1.f, top = 1.f;
            const float halfFovy = _verticalFOV * 0.5f;
            const float y = 1.f/tanf(halfFovy);
            const float x = y / aspect;
            const float scalex = 2.f * 1.f; // 1.f is sensor enlarge x
            const float scaley = 2.f * 1.f; // 1.f is sensor enlarge y
            const float dx = 0.f * 2.f * aspect / sensor.apertureHeight(); // 0.f is sensor shift x
            const float dy = 0.f * 2.f / sensor.apertureHeight();          // 0.f is sensor shift y

            m44f result;
            memset(&result, 0, sizeof(m44f));
            result.columns[0].x = scalex * x / (right - left);
            result.columns[1].y = scaley * y / (top - bottom);
            result.columns[2].x = (right + left + dx) / (right - left);
            result.columns[2].y = (top + bottom + dy) / (top - bottom);
            result.columns[2].z = handedness * (_zfar + _znear) / (_zfar - _znear);
            result.columns[2].w = handedness;
            result.columns[3].z = handedness * 2.f * _zfar * _znear / (_zfar - _znear);
            return result;
        }

    private:
        void update() {
            _verticalFOV = 2.f * atanf(sensor.apertureHeight() / (2.f * _focalLength));
        }

        Sensor sensor;
        float _zfar, _znear;
        Radians _verticalFOV;
        MM _focalLength;
    };

    class Mount {
    public:
        Mount()
        : _viewTransform(m44f_identity) {}

        void setViewTransform(const m44f & t) {
            _viewTransform = t;
        }
        const m44f& viewTransform() const { return _viewTransform; }

        v3f right()   const { return vector_normalize(V3F(_viewTransform.columns[0].x,
                                                          _viewTransform.columns[1].x,
                                                          _viewTransform.columns[2].x)); }
        v3f up()      const { return vector_normalize(V3F(_viewTransform.columns[0].y,
                                                          _viewTransform.columns[1].y,
                                                          _viewTransform.columns[2].y)); }
        v3f forward() const { return vector_normalize(V3F(_viewTransform.columns[0].z,
                                                          _viewTransform.columns[1].z,
                                                          _viewTransform.columns[2].z)); }

        static m44f lookat(v3f eye, v3f target, v3f up) {
            v3f zaxis = vector_normalize(eye - target);
            v3f xaxis = vector_normalize(vector_cross(up, zaxis));
            v3f yaxis = vector_cross(zaxis, xaxis);
            m44f ret = { V4F(xaxis.x, yaxis.x, zaxis.x, 0),
				         V4F(xaxis.y, yaxis.y, zaxis.y, 0),
					     V4F(xaxis.z, yaxis.z, zaxis.z, 0),
					     V4F(-vector_dot(xaxis, eye), -vector_dot(yaxis, eye), -vector_dot(zaxis, eye), 1 )};
            return ret;
        }

        m44f jacobian() const {
            m44f j = _viewTransform;
            j.columns[3] = {0,0,0,1};
            return j;
        }

        m44f _viewTransform;
    };

    class Camera {
    public:
        Mount mount;
        Optics optics;

        LR_API Camera()
        {
            updateViewTransform();
        }

        // Creates a matrix suitable for an OpenGL style MVP matrix
        // invert the view transform for graphics engines that pre-multiply
        //
        void updateViewTransform() {
            mount.setViewTransform(Mount::lookat(position, focusPoint, worldUp));
        }

        LR_API void frame(std::pair<v3f, v3f> bounds);
        LR_API void autoSetClippingPlanes(std::pair<v3f, v3f> bounds);

		v3f position{ 0,0,0 };
		v3f worldUp{ 0,1,0 };
		v3f focusPoint{ 0,0,-10 };
    };

    class CameraRig
    {
    public:
        enum class Mode {
            Dolly, Crane, TurnTableOrbit, Fly
        };

        void set_mode(Mode m) { _mode = m; }
        Mode mode() const { return _mode; }

        // delta is the 2d motion of a mouse or gesture in the screen plane,
        // typically computed as scale * (currMousePos - prevMousePos);
        //
        LR_API void interact(Camera * camera, v2f delta);

    private:
        Mode _mode = Mode::TurnTableOrbit;
    };



} // Lab
