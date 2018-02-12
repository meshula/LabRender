//
//  Types.h
//  LabRender
//
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#include <LabRender/Export.h>

#ifdef __APPLE__
#include <simd/simd.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <utility>
#include <ostream>

namespace lab {

    typedef float MM;
    typedef float Radians;

	inline float radToDeg(float rad)
	{
		return 180.f * rad / float(M_PI);
	}
	inline float degToRad(float deg)
	{
		return float(M_PI) * deg / 180.f;
	}

#ifdef __APPLE__

    typedef simd::int2 v2i;
    typedef simd::int4 v4i;
    typedef simd::float2 v2f;
    typedef simd::float3 v3f;
    typedef simd::float4 v4f;
    typedef matrix_float4x4 m44f;

#else

    struct v2i {
        int32_t x, y;
		    v2i() : x(), y() {}
		    v2i(int32_t x, int32_t y) : x(x), y(y) {}
		    v2i(const v2i &xy) : x(xy.x), y(xy.y) {}
		    explicit v2i(int32_t i) : x(i), y(i) {}
    };
    
    struct v4i {
        int32_t x, y, z, w;
    };

    struct v2f {
        union {
            struct { float x, y; };
            struct { float s, t; };
            struct { float r, g; };
            float xy[2];
            float st[2];
            float rg[2];
        };

        v2f() : x(), y() {}
        v2f(float x, float y) : x(x), y(y) {}
        v2f(const v2f &xy) : x(xy.x), y(xy.y) {}
        explicit v2f(float f) : x(f), y(f) {}

        v2f operator + () const { return v2f(+x, +y); }
        v2f operator - () const { return v2f(-x, -y); }

        v2f operator + (const v2f &vec) const { return v2f(x + vec.x, y + vec.y); }
        v2f operator - (const v2f &vec) const { return v2f(x - vec.x, y - vec.y); }
        v2f operator * (const v2f &vec) const { return v2f(x * vec.x, y * vec.y); }
        v2f operator / (const v2f &vec) const { return v2f(x / vec.x, y / vec.y); }
        v2f operator + (float s) const { return v2f(x + s, y + s); }
        v2f operator - (float s) const { return v2f(x - s, y - s); }
        v2f operator * (float s) const { return v2f(x * s, y * s); }
        v2f operator / (float s) const { return v2f(x / s, y / s); }

        friend v2f operator + (float s, const v2f &vec) { return v2f(s + vec.x, s + vec.y); }
        friend v2f operator - (float s, const v2f &vec) { return v2f(s - vec.x, s - vec.y); }
        friend v2f operator * (float s, const v2f &vec) { return v2f(s * vec.x, s * vec.y); }
        friend v2f operator / (float s, const v2f &vec) { return v2f(s / vec.x, s / vec.y); }

        v2f &operator += (const v2f &vec) { return *this = *this + vec; }
        v2f &operator -= (const v2f &vec) { return *this = *this - vec; }
        v2f &operator *= (const v2f &vec) { return *this = *this * vec; }
        v2f &operator /= (const v2f &vec) { return *this = *this / vec; }
        v2f &operator += (float s) { return *this = *this + s; }
        v2f &operator -= (float s) { return *this = *this - s; }
        v2f &operator *= (float s) { return *this = *this * s; }
        v2f &operator /= (float s) { return *this = *this / s; }

        bool operator == (const v2f &vec) const { return x == vec.x && y == vec.y; }
        bool operator != (const v2f &vec) const { return x != vec.x || y != vec.y; }

        friend float length(const v2f &v) { return sqrtf(v.x * v.x + v.y * v.y); }
        friend float dot(const v2f &a, const v2f &b) { return a.x * b.x + a.y * b.y; }
        friend float max(const v2f &v) { return fmaxf(v.x, v.y); }
        friend float min(const v2f &v) { return fminf(v.x, v.y); }
        friend v2f max(const v2f &a, const v2f &b) { return v2f(fmaxf(a.x, b.x), fmaxf(a.y, b.y)); }
        friend v2f min(const v2f &a, const v2f &b) { return v2f(fminf(a.x, b.x), fminf(a.y, b.y)); }
        friend v2f floor(const v2f &v) { return v2f(floorf(v.x), floorf(v.y)); }
        friend v2f ceil(const v2f &v) { return v2f(ceilf(v.x), ceilf(v.y)); }
        friend v2f abs(const v2f &v) { return v2f(fabsf(v.x), fabsf(v.y)); }
        friend v2f fract(const v2f &v) { return v - floor(v); }
        friend v2f normalized(const v2f &v) { return v / length(v); }

        friend std::ostream &operator << (std::ostream &out, const v2f &v) {
            return out << "v2f(" << v.x << ", " << v.y << ")";
        }
    };

    struct v3f {
        union {
			glm::vec3 glmV3f;
            struct { float x, y, z; };
            struct { float s, t, p; };
            struct { float r, g, b; };
            float xyz[3];
            float stp[3];
            float rgb[3];
        };

        v3f() : x(), y(), z() {}
        v3f(float x, float y, float z) : x(x), y(y), z(z) {}
        v3f(const v2f &xy, float z) : x(xy.x), y(xy.y), z(z) {}
        v3f(float x, const v2f &yz) : x(x), y(yz.x), z(yz.y) {}
        v3f(const v3f &xyz) : x(xyz.x), y(xyz.y), z(xyz.z) {}
        explicit v3f(float f) : x(f), y(f), z(f) {}

        v3f operator + () const { return v3f(+x, +y, +z); }
        v3f operator - () const { return v3f(-x, -y, -z); }

        v3f operator + (const v3f &vec) const { return v3f(x + vec.x, y + vec.y, z + vec.z); }
        v3f operator - (const v3f &vec) const { return v3f(x - vec.x, y - vec.y, z - vec.z); }
        v3f operator * (const v3f &vec) const { return v3f(x * vec.x, y * vec.y, z * vec.z); }
        v3f operator / (const v3f &vec) const { return v3f(x / vec.x, y / vec.y, z / vec.z); }
        v3f operator + (float s) const { return v3f(x + s, y + s, z + s); }
        v3f operator - (float s) const { return v3f(x - s, y - s, z - s); }
        v3f operator * (float s) const { return v3f(x * s, y * s, z * s); }
        v3f operator / (float s) const { return v3f(x / s, y / s, z / s); }

        friend v3f operator + (float s, const v3f &vec) { return v3f(s + vec.x, s + vec.y, s + vec.z); }
        friend v3f operator - (float s, const v3f &vec) { return v3f(s - vec.x, s - vec.y, s - vec.z); }
        friend v3f operator * (float s, const v3f &vec) { return v3f(s * vec.x, s * vec.y, s * vec.z); }
        friend v3f operator / (float s, const v3f &vec) { return v3f(s / vec.x, s / vec.y, s / vec.z); }

        v3f &operator += (const v3f &vec) { return *this = *this + vec; }
        v3f &operator -= (const v3f &vec) { return *this = *this - vec; }
        v3f &operator *= (const v3f &vec) { return *this = *this * vec; }
        v3f &operator /= (const v3f &vec) { return *this = *this / vec; }
        v3f &operator += (float s) { return *this = *this + s; }
        v3f &operator -= (float s) { return *this = *this - s; }
        v3f &operator *= (float s) { return *this = *this * s; }
        v3f &operator /= (float s) { return *this = *this / s; }

        bool operator == (const v3f &vec) const { return x == vec.x && y == vec.y && z == vec.z; }
        bool operator != (const v3f &vec) const { return x != vec.x || y != vec.y || z != vec.z; }

        friend float length(const v3f &v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
        friend float dot(const v3f &a, const v3f &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
        friend float max(const v3f &v) { return fmaxf(fmaxf(v.x, v.y), v.z); }
        friend float min(const v3f &v) { return fminf(fminf(v.x, v.y), v.z); }
        friend v3f max(const v3f &a, const v3f &b) { return v3f(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z)); }
        friend v3f min(const v3f &a, const v3f &b) { return v3f(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z)); }
        friend v3f floor(const v3f &v) { return v3f(floorf(v.x), floorf(v.y), floorf(v.z)); }
        friend v3f ceil(const v3f &v) { return v3f(ceilf(v.x), ceilf(v.y), ceilf(v.z)); }
        friend v3f abs(const v3f &v) { return v3f(fabsf(v.x), fabsf(v.y), fabsf(v.z)); }
        friend v3f fract(const v3f &v) { return v - floor(v); }
        friend v3f normalized(const v3f &v) { return v / length(v); }
        friend v3f cross(const v3f &a, const v3f &b) { return v3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }

        friend std::ostream &operator << (std::ostream &out, const v3f &v) {
            return out << "v3f(" << v.x << ", " << v.y << ", " << v.z << ")";
        }
    };

    struct v4f {
        union {
            struct { float x, y, z, w; };
            struct { float r, g, b, a; };
			struct { v3f xyz; float w; };
            float xyzw[4];
            float rgba[4];
        };

        v4f() : x(), y(), z(), w() {}
        v4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        v4f(const v2f &xy, float z, float w) : x(xy.x), y(xy.y), z(z), w(w) {}
        v4f(float x, const v2f &yz, float w) : x(x), y(yz.x), z(yz.y), w(w) {}
        v4f(float x, float y, const v2f &zw) : x(x), y(y), z(zw.x), w(zw.y) {}
        v4f(const v2f &xy, const v2f &zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
        v4f(const v3f &xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
        v4f(float x, const v3f &yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}
        v4f(const v4f &xyzw) : x(xyzw.x), y(xyzw.y), z(xyzw.z), w(xyzw.w) {}
        explicit v4f(float f) : x(f), y(f), z(f), w(f) {}

        v4f operator + () const { return v4f(+x, +y, +z, +w); }
        v4f operator - () const { return v4f(-x, -y, -z, -w); }

        v4f operator + (const v4f &vec) const { return v4f(x + vec.x, y + vec.y, z + vec.z, w + vec.w); }
        v4f operator - (const v4f &vec) const { return v4f(x - vec.x, y - vec.y, z - vec.z, w - vec.w); }
        v4f operator * (const v4f &vec) const { return v4f(x * vec.x, y * vec.y, z * vec.z, w * vec.w); }
        v4f operator / (const v4f &vec) const { return v4f(x / vec.x, y / vec.y, z / vec.z, w / vec.w); }
        v4f operator + (float s) const { return v4f(x + s, y + s, z + s, w + s); }
        v4f operator - (float s) const { return v4f(x - s, y - s, z - s, w - s); }
        v4f operator * (float s) const { return v4f(x * s, y * s, z * s, w * s); }
        v4f operator / (float s) const { return v4f(x / s, y / s, z / s, w / s); }

        friend v4f operator + (float s, const v4f &vec) { return v4f(s + vec.x, s + vec.y, s + vec.z, s + vec.w); }
        friend v4f operator - (float s, const v4f &vec) { return v4f(s - vec.x, s - vec.y, s - vec.z, s - vec.w); }
        friend v4f operator * (float s, const v4f &vec) { return v4f(s * vec.x, s * vec.y, s * vec.z, s * vec.w); }
        friend v4f operator / (float s, const v4f &vec) { return v4f(s / vec.x, s / vec.y, s / vec.z, s / vec.w); }

        v4f &operator += (const v4f &vec) { return *this = *this + vec; }
        v4f &operator -= (const v4f &vec) { return *this = *this - vec; }
        v4f &operator *= (const v4f &vec) { return *this = *this * vec; }
        v4f &operator /= (const v4f &vec) { return *this = *this / vec; }
        v4f &operator += (float s) { return *this = *this + s; }
        v4f &operator -= (float s) { return *this = *this - s; }
        v4f &operator *= (float s) { return *this = *this * s; }
        v4f &operator /= (float s) { return *this = *this / s; }

        bool operator == (const v4f &vec) const { return x == vec.x && y == vec.y && z == vec.z && w == vec.w; }
        bool operator != (const v4f &vec) const { return x != vec.x || y != vec.y || z != vec.z || w != vec.w; }

        friend float length(const v4f &v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
        friend float dot(const v4f &a, const v4f &b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * a.w; }
        friend float max(const v4f &v) { return fmaxf(fmaxf(v.x, v.y), fmaxf(v.z, v.w)); }
        friend float min(const v4f &v) { return fminf(fminf(v.x, v.y), fminf(v.z, v.w)); }
        friend v4f max(const v4f &a, const v4f &b) { return v4f(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z), fmaxf(a.w, b.w)); }
        friend v4f min(const v4f &a, const v4f &b) { return v4f(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z), fminf(a.w, b.w)); }
        friend v4f floor(const v4f &v) { return v4f(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w)); }
        friend v4f ceil(const v4f &v) { return v4f(ceilf(v.x), ceilf(v.y), ceilf(v.z), ceilf(v.w)); }
        friend v4f abs(const v4f &v) { return v4f(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w)); }
        friend v4f fract(const v4f &v) { return v - floor(v); }
        friend v4f normalized(const v4f &v) { return v / length(v); }

        friend std::ostream &operator << (std::ostream &out, const v4f &v) {
            return out << "v4f(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        }
    };

    struct m44f {
        union {
            struct {
                float m00, m01, m02, m03;
                float m10, m11, m12, m13;
                float m20, m21, m22, m23;
                float m30, m31, m32, m33;
            };
			struct {
				v4f columns[4];
			};
            float m[16];
        };

        m44f() :
			m00(1), m01(), m02(), m03(),
			m10(), m11(1), m12(), m13(),
			m20(), m21(), m22(1), m23(),
			m30(), m31(), m32(), m33(1) {}
        m44f(const v4f &r0, const v4f &r1, const v4f &r2, const v4f &r3) :
			m00(r0.x), m01(r0.y), m02(r0.z), m03(r0.w),
			m10(r1.x), m11(r1.y), m12(r1.z), m13(r1.w),
			m20(r2.x), m21(r2.y), m22(r2.z), m23(r2.w),
			m30(r3.x), m31(r3.y), m32(r3.z), m33(r3.w) {}
        m44f(float m00, float m01, float m02, float m03,
             float m10, float m11, float m12, float m13,
             float m20, float m21, float m22, float m23,
             float m30, float m31, float m32, float m33) :
				m00(m00), m01(m01), m02(m02), m03(m03),
				m10(m10), m11(m11), m12(m12), m13(m13),
				m20(m20), m21(m21), m22(m22), m23(m23),
				m30(m30), m31(m31), m32(m32), m33(m33) {}
		m44f(const m44f & rh) {
			columns[0] = rh.columns[0];
			columns[1] = rh.columns[1];
			columns[2] = rh.columns[2];
			columns[3] = rh.columns[3];
		}

        m44f &transpose();
        m44f &rotateX(float degrees);
        m44f &rotateY(float degrees);
        m44f &rotateZ(float degrees);
        m44f &rotate(float degrees, float x, float y, float z);
        m44f &rotate(float degrees, const v3f &v) { return rotate(degrees, v.x, v.y, v.z); }
        m44f &scale(float x, float y, float z);
        m44f &scale(const v3f &v) { return scale(v.x, v.y, v.z); }
        m44f &translate(float x, float y, float z);
        m44f &translate(const v3f &v) { return translate(v.x, v.y, v.z); }
        m44f &ortho(float l, float r, float b, float t, float n, float f);
        m44f &frustum(float l, float r, float b, float t, float n, float f);
        m44f &perspective(float fov, float aspect, float near, float far);
        m44f &invert();

        LR_API void decompose(v3f& t, v3f& r, v3f& s) const;
        LR_API void recompose(const v3f& t, const v3f& r, const v3f& s);
        LR_API void orthonormalize();

		m44f & operator = (const m44f & a) {
			if (&a != this) { memcpy(this, &a, sizeof(m44f)); }
			return *this;
		}
        m44f & operator *= (const m44f &t);
        v4f operator * (const v4f &v);
        friend v4f operator * (const v4f &v, const m44f &t);
        friend std::ostream &operator << (std::ostream &out, const m44f &t);
    };

	LR_CAPI m44f m44f_identity;

	template <typename T> inline float vector_length(const T & a) { return length(a); }
	template <typename T> inline T vector_normalize(const T & a) { return normalized(a); }
	template <typename T> inline float vector_dot(const T & a, const T & b) { return dot(a, b); }

	inline v3f vector_cross(const v3f & a, const v3f & b) { return cross(a, b); }
	LR_API v4f matrix_multiply(const m44f & a, const v4f & v);
	LR_API m44f matrix_multiply(const m44f & a, const m44f & b);

	LR_API m44f matrix_invert(const m44f & a);
	LR_API m44f matrix_transpose(const m44f & a);

#endif

    typedef std::pair<v3f, v3f> Bounds;

    inline Bounds extendBounds(Bounds bounds, v3f xyz) {
        if (xyz.x < bounds.first.x) bounds.first.x = xyz.x;
        if (xyz.y < bounds.first.y) bounds.first.y = xyz.y;
        if (xyz.z < bounds.first.z) bounds.first.z = xyz.z;
        if (xyz.x > bounds.second.x) bounds.second.x = xyz.x;
        if (xyz.y > bounds.second.y) bounds.second.y = xyz.y;
        if (xyz.z > bounds.second.z) bounds.second.z = xyz.z;
        return bounds;
    }
    inline Bounds extendBounds(Bounds bounds, const Bounds& xyz) {
        Bounds ret = extendBounds(bounds, xyz.first);
        return extendBounds(ret, xyz.second);
    }


}


typedef glm::fquat quatf;

#ifdef __APPLE__
#define M44F(c1x, c1y, c1z, c1w, c2x, c2y, c2z, c2w, c3x, c3y, c3z, c3w, c4x, c4y, c4z, c4w) \
  { (v4f){float(c1x), float(c1y), float(c1z), float(c1w)}, \
    (v4f){float(c2x), float(c2y), float(c2z), float(c2w)}, \
    (v4f){float(c3x), float(c3y), float(c3z), float(c3w)}, \
    (v4f){float(c4x), float(c4y), float(c4z), float(c4w)}  }
#define V2I(x, y) (v2f){float(x), float(y)}
#define V2F(x, y) (v2f){float(x), float(y)}
#define V3F(x, y, z) (v3f){float(x), float(y), float(z)}
#define V4F(x, y, z, w) (v4f){float(x), float(y), float(z), float(w)}
#else
#define M44F(c1x, c1y, c1z, c1w, c2x, c2y, c2z, c2w, c3x, c3y, c3z, c3w, c4x, c4y, c4z, c4w) \
  { float(c1x), float(c1y), float(c1z), float(c1w), \
    float(c2x), float(c2y), float(c2z), float(c2w), \
    float(c3x), float(c3y), float(c3z), float(c3w), \
    float(c4x), float(c4y), float(c4z), float(c4w)  }
#define V2I(x, y) v2i(int32_t(x), int32_t(y))
#define V2F(x, y) v2f(float(x), float(y))
#define V3F(x, y, z) v3f(float(x), float(y), float(z))
#define V4F(x, y, z, w) v4f(float(x), float(y), float(z), float(w))
#endif
