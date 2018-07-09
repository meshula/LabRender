
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LabRender/Camera.h"
#include <LabMath/LabMath.h>

namespace lab
{
    LR_API m44f perspective(const Sensor& sensor, const Optics& optics, float aspect)
    {
        if (fabs(aspect) < std::numeric_limits<float>::epsilon())
            return m44f_identity;

        const float handedness = sensor.handedness; // 1 for left hand coordinates
        float left = -1.f, right = 1.f, bottom = -1.f, top = 1.f;
        const float halfFovy = verticalFOV(sensor, optics) * 0.5f;
        const float y = 1.f/tanf(halfFovy);
        const float x = y / aspect;
        const float scalex = 2.f * sensor.enlarge.x;
        const float scaley = 2.f * sensor.enlarge.y;
        const float dx = sensor.shift.x * 2.f * aspect / sensor.aperture.y; // 0.f is sensor shift x
        const float dy = sensor.shift.y * 2.f / sensor.aperture.y;          // 0.f is sensor shift y

        const float znear = optics.znear;
        const float zfar = optics.zfar;

        m44f result;
        memset(&result, 0, sizeof(m44f));
        result[0].x = scalex * x / (right - left);
        result[1].y = scaley * y / (top - bottom);
        result[2].x = (right + left + dx) / (right - left);
        result[2].y = (top + bottom + dy) / (top - bottom);
        result[2].z = handedness * (zfar + znear) / (zfar - znear);
        result[2].w = handedness;
        result[3].z = handedness * 2.f * zfar * znear / (zfar - znear);
        return result;
    }

    LR_API m44f perspective(const Sensor& sensor, const Optics& optics) 
    {
        const float aspect = sensor.aperture.x / sensor.aperture.y;
        return perspective(sensor, optics, aspect);
    }

    void Camera::frame(std::pair<v3f, v3f> bounds)
	{
        float r = 0.5f * vector_length(bounds.second - bounds.first);
        float g = (1.1f * r) / sinf(verticalFOV(sensor, optics) * 0.5f);
        focusPoint = (bounds.second + bounds.first) * 0.5f;
        position = vector_normalize(position - focusPoint) * g;
        updateViewTransform();
    }

    void Camera::autoSetClippingPlanes(std::pair<v3f, v3f> bounds)
	{
        float clipNear = FLT_MAX;
        float clipFar = FLT_MIN;

        v4f points[8] = {
            {bounds.first.x,  bounds.first.y,  bounds.first.z,  1.f},
            {bounds.first.x,  bounds.first.y,  bounds.second.z, 1.f},
            {bounds.first.x,  bounds.second.y, bounds.first.z,  1.f},
            {bounds.first.x,  bounds.second.y, bounds.second.z, 1.f},
            {bounds.second.x, bounds.first.y,  bounds.first.z,  1.f},
            {bounds.second.x, bounds.first.y,  bounds.second.z, 1.f},
            {bounds.second.x, bounds.second.y, bounds.first.z,  1.f},
            {bounds.second.x, bounds.second.y, bounds.second.z, 1.f} };

        for (int p = 0; p < 8; ++p) {
            v4f dp = matrix_multiply(mount.viewTransform(), points[p]);
            clipNear = std::min(dp.z, clipNear);
            clipFar = std::max(dp.z, clipFar);
        }

        clipNear -= 0.5f;
        clipFar += 0.5f;
        clipNear = std::max(0.1f, std::min(clipNear, 100000.f));
        clipFar = std::max(clipNear, std::min(clipNear, 100000.f));

        if (clipFar <= clipNear)
            clipFar = clipNear + 0.1f;

        optics.znear = clipNear;
        optics.zfar = clipFar;
    }


    // delta is the 2d motion of a mouse or gesture in the screen plane,
    // typically computed as scale * (currMousePos - prevMousePos);
    //
    void cameraRig_interact(Camera& camera, CameraRigMode mode, v2f delta)
	{
        v3f cameraToFocus = camera.position - camera.focusPoint;
        float distanceToFocus = vector_length(cameraToFocus);
        float scale = std::max(0.01f, logf(distanceToFocus) * 4.f);

        switch (mode) 
        {
            case CameraRigMode::Dolly: 
            {
                v3f camFwd = camera.mount.forward();
                v3f camRight = camera.mount.right();
                v3f deltaX = delta.x * camRight * scale;
                v3f dP = delta.y * camFwd * scale - deltaX;
                camera.position += dP;
                camera.focusPoint -= deltaX;
                break;
            }
            case CameraRigMode::Crane: 
            {
                v3f camUp = camera.mount.up();
                v3f camRight = camera.mount.right();
                v3f dP = -delta.y * camUp * scale - delta.x * camRight * scale;
                camera.position += dP;
                camera.focusPoint += dP;
                break;
            }
            case CameraRigMode::TurnTableOrbit: 
            {
                v3f camFwd = camera.mount.forward();
				v3f worldUp = camera.worldUp;
                v3f mUv = vector_normalize(vector_cross(worldUp, camFwd));
				quatf yaw = quat_fromAxisAngle(v3f(0,1.f,0), -0.25f * delta.x);
				quatf pitch = quat_fromAxisAngle(mUv, 0.25f * delta.y);
				v3f rotatedVec = quat_rotateVector(yaw, quat_rotateVector(pitch, cameraToFocus));
				camera.position = camera.focusPoint + rotatedVec;
                break;
            }
        }
        camera.updateViewTransform();
    }




} // lab





#if 0
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <simd/simd.h>

namespace LabRender {

    typedef glm::vec3 V3d;
    typedef glm::vec2 V2d;
    typedef glm::mat4x4 M44d;

    static inline void rotateVector( double rx, double ry, V3d &v )
    {
        const double sinX = sinf( rx );
        const double cosX = cosf( rx );

        const V3d t( v.x,
                    ( v.y * cosX ) - ( v.z * sinX ),
                    ( v.y * sinX ) + ( v.z * cosX ) );

        const double sinY = sinf( ry );
        const double cosY = cosf( ry );

        v.x = ( t.x * cosY ) + ( t.z * sinY );
        v.y = t.y;
        v.z = ( t.x * -sinY ) + ( t.z * cosY );
    }

    Camera::Camera()
    : _sensoraperture.y(24.5f)
    , _focalLength(50.f)
    , _clip(0.1f, 1000.f)
    {
        calcFovy();
    }

    void Camera::calcFovy() {
        _fovyRad = 2.f * atanf(_sensoraperture.y / (2.f * _focalLength));
    }

    glm::mat4x4 Camera::perspective(float w, float h) const
    {
        float aspect = w/h;
        if (fabs(aspect) < std::numeric_limits<float>::epsilon())
            return glm::mat4x4(1);

        float const tanHalfFovy = tanf(_fovyRad * 0.5f);
        const float handedness = -1.f; // 1 for left hand coordinates

        matrix_float4x4 result;
        memset(&result, 0, sizeof(matrix_float4x4));

        result.columns[0].x = 1.f / (aspect * tanHalfFovy);
        result.columns[1].y = 1.f / tanHalfFovy;
        result.columns[2].z = handedness * (_clip.y + _clip.x) / (_clip.y - _clip.x);
        result.columns[2].w = handedness;
        result.columns[3].z = -2.f * _clip.y * _clip.x / (_clip.y - _clip.x);

        glm::mat4x4 res;
        memcpy(&res, &result, sizeof(result));
        return res;
    }

    UICamera::UICamera()
    : lookatConstraintActive(false) { }

    void UICamera::updateConstraints() {
        if (lookatConstraintActive)
            transform.setView(lookat, glm::vec3(0,1,0));
    }


    void UICamera::track(const glm::vec2 &point)
    {
        // INIT
        V3d rot = transform.ypr();
        const double rotX = rot.y;
        const double rotY = rot.x;

        V3d dS( 1.0, 0.0, 0.0 );
        rotateVector( rotX, rotY, dS );

        V3d dT( 0.0, 1.0, 0.0 );
        rotateVector( rotX, rotY, dT );

        double multS = 2.0 * _interestDistance * tanf(_fovyRad) / 2.0f;
        const double multT = multS / double( height() );
        multS /= double( width() );

        // TRACK
        const float s = -multS * point.x;
        const float t = multT * point.y;

        // ALTER
        V3d translate = transform.translate() + ( s * dS ) + ( t * dT );
        transform.setTranslate(translate);
    }

    void UICamera::dolly(const V2d &point, double dollySpeed) {
        // INIT
        V3d rot = transform.ypr();
        const double rotX = rot.y;
        const double rotY = rot.x;
        const V3d eye = transform.translate();

        V3d v(0.0, 0.0, -_interestDistance);
        rotateVector( rotX, rotY, v );
        const V3d view = eye + v;
        v = glm::normalize(v);

        // DOLLY
        const double t = point.x / double( width() );

        // Magic dolly function
        float dollyBy = 1.0f - expf( -dollySpeed * t );

        if (dollyBy > 1.0f)
        {
            dollyBy = 1.0f;
        }
        else if (dollyBy < -1.0f)
        {
            dollyBy = -1.0f;
        }

        dollyBy *= _interestDistance;
        const V3d newEye = eye + (v * dollyBy);

        // ALTER
        transform.setTranslate(newEye);
        v = newEye - view;
        _interestDistance = length(v);
    }

    void UICamera::orbit(const V2d &point, double rotateSpeed) {
        // INIT
        V3d rot = transform.ypr();
        double rotX = rot.y;
        double rotY = rot.x;
        const double rotZ = rot.z;
        const V3d eye = transform.translate();

        V3d v(0.0, 0.0, -_interestDistance);
        rotateVector( rotX, rotY, v );

        const V3d view = eye + v;

        // ROTATE
        rotY += -rotateSpeed * ( point.x / double( width() ) );
        rotX += -rotateSpeed * ( point.y / double( height() ) );

        v[0] = 0.0;
        v[1] = 0.0;
        v[2] = _interestDistance;
        rotateVector( rotX, rotY, v );

        // ALTER
        transform.setTranslate(view + v);
        transform.setYpr(V3d(rotY, rotX, rotZ));
    }

    void UICamera::frame(const glm::vec3& minExtent, const glm::vec3& maxExtent) {
        float r = 0.5f * length(maxExtent - minExtent);
        float g = ( 1.1f * r ) / sinf(_fovyRad * 0.5f);
        glm::vec3 center = (minExtent + maxExtent) * 0.5f;
        glm::vec3 cameraVector = normalize(transform.translate() - center) * g;
        lookAt(cameraVector, center);
    }

    void UICamera::lookAt( const V3d &eye, const V3d &at ) {
        transform.setTranslate(eye);

        // calculate as if eye is at 0,0,0
        const V3d lookAtPoint = at - eye;

        const double xzLen = sqrt( (lookAtPoint.x * lookAtPoint.x) +
                                   (lookAtPoint.z * lookAtPoint.z) );

        float azimuthalAngle = atan2(lookAtPoint.z, lookAtPoint.x); // yaw
        float polarAngle = atan2(xzLen, lookAtPoint.y); // pitch

        float rotPitch = polarAngle;// = atan2( lookAtPoint.y, xzLen );
        float rotYaw = azimuthalAngle;//atan2( lookAtPoint.x, -lookAtPoint.z );
        float rotZ = transform.ypr().z;         // should be based on an up vector

        transform.setYpr(V3d(rotYaw, rotPitch, rotZ));
        _interestDistance = length(lookAtPoint);
    }

    void UICamera::autoSetClippingPlanes(const glm::vec3& minExtent, const glm::vec3& maxExtent) {
        V3d rot = transform.ypr();
        double rotX = rot.y;
        double rotY = rot.x;
        const V3d eye = transform.translate();
        double clipNear = FLT_MAX;
        double clipFar = FLT_MIN;

        V3d v(0.0, 0.0, - _interestDistance);
        rotateVector( rotX, rotY, v );
        //const V3d view = eye + v;
        v = glm::normalize(v);

        V3d points[8];

        points[0] = V3d( minExtent.x, minExtent.y, minExtent.z );
        points[1] = V3d( minExtent.x, minExtent.y, maxExtent.z );
        points[2] = V3d( minExtent.x, maxExtent.y, minExtent.z );
        points[3] = V3d( minExtent.x, maxExtent.y, maxExtent.z );
        points[4] = V3d( maxExtent.x, minExtent.y, minExtent.z );
        points[5] = V3d( maxExtent.x, minExtent.y, maxExtent.z );
        points[6] = V3d( maxExtent.x, maxExtent.y, minExtent.z );
        points[7] = V3d( maxExtent.x, maxExtent.y, maxExtent.z );

        for( int p = 0; p < 8; ++p )
        {
            V3d dp = points[p] - eye;
            double proj = glm::dot(dp, v);
            clipNear = std::min( proj, clipNear );
            clipFar = std::max( proj, clipFar );
        }

        clipNear -= 0.5f;
        clipFar += 0.5f;
        clipNear = glm::clamp( clipNear, 0.1, 100000.0 );
        clipFar = glm::clamp( clipFar, 0.1, 100000.0 );

        if ( clipFar <= clipNear )
        {
            clipFar = clipNear + 0.1;
        }

        _clip.x = clipNear;
        _clip.y = clipFar;
    }

} // Lab
#endif
