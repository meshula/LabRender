#include "LabRender/gl4.h"
#include "LabRender/MathTypes.h"

#include <exception>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace lab;

#ifndef __APPLE__
m44f &m44f::transpose() {
    std::swap(m01, m10); std::swap(m02, m20); std::swap(m03, m30);
    std::swap(m12, m21); std::swap(m13, m31); std::swap(m23, m32);
    return *this;
}

m44f &m44f::rotateX(float degrees) {
    float radians = degrees * float(M_PI / 180);
    float s = sinf(radians), c = cosf(radians);
    float t01 = m01, t02 = m02;
    float t11 = m11, t12 = m12;
    float t21 = m21, t22 = m22;
    float t31 = m31, t32 = m32;
    m01 = c * t01 - s * t02;
    m02 = c * t02 + s * t01;
    m11 = c * t11 - s * t12;
    m12 = c * t12 + s * t11;
    m21 = c * t21 - s * t22;
    m22 = c * t22 + s * t21;
    m31 = c * t31 - s * t32;
    m32 = c * t32 + s * t31;
    return *this;
}

m44f &m44f::rotateY(float degrees) {
    float radians = degrees * float(M_PI / 180);
    float s = sinf(radians), c = cosf(radians);
    float t02 = m02, t00 = m00;
    float t12 = m12, t10 = m10;
    float t22 = m22, t20 = m20;
    float t32 = m32, t30 = m30;
    m02 = c * t02 - s * t00;
    m00 = c * t00 + s * t02;
    m12 = c * t12 - s * t10;
    m10 = c * t10 + s * t12;
    m22 = c * t22 - s * t20;
    m20 = c * t20 + s * t22;
    m32 = c * t32 - s * t30;
    m30 = c * t30 + s * t32;
    return *this;
}

m44f &m44f::rotateZ(float degrees) {
    float radians = degrees * float(M_PI / 180);
    float s = sinf(radians), c = cosf(radians);
    float t00 = m00, t01 = m01;
    float t10 = m10, t11 = m11;
    float t20 = m20, t21 = m21;
    float t30 = m30, t31 = m31;
    m00 = c * t00 - s * t01;
    m01 = c * t01 + s * t00;
    m10 = c * t10 - s * t11;
    m11 = c * t11 + s * t10;
    m20 = c * t20 - s * t21;
    m21 = c * t21 + s * t20;
    m30 = c * t30 - s * t31;
    m31 = c * t31 + s * t30;
    return *this;
}

m44f &m44f::rotate(float degrees, float x, float y, float z) {
    float radians = degrees * float(M_PI / 180);
    float d = sqrtf(x*x + y*y + z*z);
    float s = sinf(radians), c = cosf(radians), t = 1 - c;
    x /= d; y /= d; z /= d;
    return *this *= m44f(
        x*x*t + c, x*y*t - z*s, x*z*t + y*s, 0,
        y*x*t + z*s, y*y*t + c, y*z*t - x*s, 0,
        z*x*t - y*s, z*y*t + x*s, z*z*t + c, 0,
        0, 0, 0, 1);
}

m44f &m44f::scale(float x, float y, float z) {
    m00 *= x; m01 *= y; m02 *= z;
    m10 *= x; m11 *= y; m12 *= z;
    m20 *= x; m21 *= y; m22 *= z;
    m30 *= x; m31 *= y; m32 *= z;
    return *this;
}

m44f &m44f::translate(float x, float y, float z) {
    m03 += m00 * x + m01 * y + m02 * z;
    m13 += m10 * x + m11 * y + m12 * z;
    m23 += m20 * x + m21 * y + m22 * z;
    m33 += m30 * x + m31 * y + m32 * z;
    return *this;
}

m44f &m44f::ortho(float l, float r, float b, float t, float n, float f) {
    return *this *= m44f(
        2 / (r - l), 0, 0, (r + l) / (l - r),
        0, 2 / (t - b), 0, (t + b) / (b - t),
        0, 0, 2 / (n - f), (f + n) / (n - f),
        0, 0, 0, 1);
}

m44f &m44f::frustum(float l, float r, float b, float t, float n, float f) {
    return *this *= m44f(
        2 * n / (r - l), 0, (r + l) / (r - l), 0,
        0, 2 * n / (t - b), (t + b) / (t - b), 0,
        0, 0, (f + n) / (n - f), 2 * f * n / (n - f),
        0, 0, -1, 0);
}

m44f &m44f::perspective(float fov, float aspect, float znear, float zfar) {
    float y = tanf(fov * float(M_PI / 360)) * znear, x = y * aspect;
    return frustum(-x, x, -y, y, znear, zfar);
}

m44f &m44f::invert() {
    float t00 = m00, t01 = m01, t02 = m02, t03 = m03;
    *this = m44f(
        m11*m22*m33 - m11*m32*m23 - m12*m21*m33 + m12*m31*m23 + m13*m21*m32 - m13*m31*m22,
        -m01*m22*m33 + m01*m32*m23 + m02*m21*m33 - m02*m31*m23 - m03*m21*m32 + m03*m31*m22,
        m01*m12*m33 - m01*m32*m13 - m02*m11*m33 + m02*m31*m13 + m03*m11*m32 - m03*m31*m12,
        -m01*m12*m23 + m01*m22*m13 + m02*m11*m23 - m02*m21*m13 - m03*m11*m22 + m03*m21*m12,

        -m10*m22*m33 + m10*m32*m23 + m12*m20*m33 - m12*m30*m23 - m13*m20*m32 + m13*m30*m22,
        m00*m22*m33 - m00*m32*m23 - m02*m20*m33 + m02*m30*m23 + m03*m20*m32 - m03*m30*m22,
        -m00*m12*m33 + m00*m32*m13 + m02*m10*m33 - m02*m30*m13 - m03*m10*m32 + m03*m30*m12,
        m00*m12*m23 - m00*m22*m13 - m02*m10*m23 + m02*m20*m13 + m03*m10*m22 - m03*m20*m12,

        m10*m21*m33 - m10*m31*m23 - m11*m20*m33 + m11*m30*m23 + m13*m20*m31 - m13*m30*m21,
        -m00*m21*m33 + m00*m31*m23 + m01*m20*m33 - m01*m30*m23 - m03*m20*m31 + m03*m30*m21,
        m00*m11*m33 - m00*m31*m13 - m01*m10*m33 + m01*m30*m13 + m03*m10*m31 - m03*m30*m11,
        -m00*m11*m23 + m00*m21*m13 + m01*m10*m23 - m01*m20*m13 - m03*m10*m21 + m03*m20*m11,

        -m10*m21*m32 + m10*m31*m22 + m11*m20*m32 - m11*m30*m22 - m12*m20*m31 + m12*m30*m21,
        m00*m21*m32 - m00*m31*m22 - m01*m20*m32 + m01*m30*m22 + m02*m20*m31 - m02*m30*m21,
        -m00*m11*m32 + m00*m31*m12 + m01*m10*m32 - m01*m30*m12 - m02*m10*m31 + m02*m30*m11,
        m00*m11*m22 - m00*m21*m12 - m01*m10*m22 + m01*m20*m12 + m02*m10*m21 - m02*m20*m11
    );
    float det = m00 * t00 + m10 * t01 + m20 * t02 + m30 * t03;
    for (int i = 0; i < 16; i++) m[i] /= det;
    return *this;
}

m44f &m44f::operator *= (const m44f &t) {
    *this = m44f(
        m00*t.m00 + m01*t.m10 + m02*t.m20 + m03*t.m30,
        m00*t.m01 + m01*t.m11 + m02*t.m21 + m03*t.m31,
        m00*t.m02 + m01*t.m12 + m02*t.m22 + m03*t.m32,
        m00*t.m03 + m01*t.m13 + m02*t.m23 + m03*t.m33,

        m10*t.m00 + m11*t.m10 + m12*t.m20 + m13*t.m30,
        m10*t.m01 + m11*t.m11 + m12*t.m21 + m13*t.m31,
        m10*t.m02 + m11*t.m12 + m12*t.m22 + m13*t.m32,
        m10*t.m03 + m11*t.m13 + m12*t.m23 + m13*t.m33,

        m20*t.m00 + m21*t.m10 + m22*t.m20 + m23*t.m30,
        m20*t.m01 + m21*t.m11 + m22*t.m21 + m23*t.m31,
        m20*t.m02 + m21*t.m12 + m22*t.m22 + m23*t.m32,
        m20*t.m03 + m21*t.m13 + m22*t.m23 + m23*t.m33,

        m30*t.m00 + m31*t.m10 + m32*t.m20 + m33*t.m30,
        m30*t.m01 + m31*t.m11 + m32*t.m21 + m33*t.m31,
        m30*t.m02 + m31*t.m12 + m32*t.m22 + m33*t.m32,
        m30*t.m03 + m31*t.m13 + m32*t.m23 + m33*t.m33
    );
    return *this;
}

v4f m44f::operator * (const v4f &v) {
    return v4f(
        m00*v.x + m01*v.y + m02*v.z + m03*v.w,
        m10*v.x + m11*v.y + m12*v.z + m13*v.w,
        m20*v.x + m21*v.y + m22*v.z + m23*v.w,
        m30*v.x + m31*v.y + m32*v.z + m33*v.w
    );
}

v4f operator * (const v4f &v, const m44f &t) {
    return v4f(
        t.m00*v.x + t.m10*v.y + t.m20*v.z + t.m30*v.w,
        t.m01*v.x + t.m11*v.y + t.m21*v.z + t.m31*v.w,
        t.m02*v.x + t.m12*v.y + t.m22*v.z + t.m32*v.w,
        t.m03*v.x + t.m13*v.y + t.m23*v.z + t.m33*v.w
    );
}

std::ostream &operator << (std::ostream &out, const m44f &t) {
    return out << "m44f("
        << t.m00 << ", " << t.m01 << ", " << t.m02 << ", " << t.m03 << ",\n     "
        << t.m10 << ", " << t.m11 << ", " << t.m12 << ", " << t.m13 << ",\n     "
        << t.m20 << ", " << t.m21 << ", " << t.m22 << ", " << t.m23 << ",\n     "
        << t.m30 << ", " << t.m31 << ", " << t.m32 << ", " << t.m33 << ")";
}

#endif

