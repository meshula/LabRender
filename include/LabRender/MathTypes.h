//
//  Types.h
//  LabRender
//
//  Copyright (c) 2015 Planet IX. All rights reserved.
//

#pragma once

#include <LabRender/Export.h>

#define _USE_MATH_DEFINES
#include <math.h>

//#define HAVE_GLM
#define HAVE_LINALG

#ifdef HAVE_GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#endif
#ifdef HAVE_LINALG
#include <LabRender/linalg.h>
#endif

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

#ifdef HAVE_GLM
    typedef glm::highp_vec2 v2f;
    typedef glm::highp_vec3 v3f;
    typedef glm::highp_vec4 v4f;
    typedef glm::highp_mat4x4 m44f;
    typedef glm::fquat quatf;
#elif defined(HAVE_LINALG)
    typedef linalg::vec<float, 2> v2f;
    typedef linalg::vec<float, 3> v3f;
    typedef linalg::vec<float, 4> v4f;
    typedef linalg::mat<float, 4, 4> m44f;
    typedef linalg::vec<float, 4> quatf;
#endif

    inline float length(const v2f &v) { return sqrtf(v.x * v.x + v.y * v.y); }
    inline float length(const v3f &v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
    inline float length(const v4f &v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

#if 0
    struct m44f {
        union {
            struct {
                float m00, m01, m02, m03;
                float m10, m11, m12, m13;
                float m20, m21, m22, m23;
                float m30, m31, m32, m33;
            };
            glm::mat4 mat4;
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
		m44f(const m44f & rh) 
        : mat4(rh.mat4)
        {}


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

#endif

    LR_API void transpose(m44f&);
    LR_API void rotateX(m44f&, float degrees);
    LR_API void rotateY(m44f&, float degrees);
    LR_API void rotateZ(m44f&, float degrees);
    LR_API void rotate(m44f&, float degrees, float x, float y, float z);
    inline void rotate(m44f& mat, float degrees, const v3f &v) { return rotate(mat, degrees, v.x, v.y, v.z); }
    LR_API void scale(m44f&, float x, float y, float z);
    inline void scale(m44f& mat, const v3f &v) { return scale(mat, v.x, v.y, v.z); }
    LR_API void translate(m44f&, float x, float y, float z);
    inline void translate(m44f& mat, const v3f &v) { return translate(mat, v.x, v.y, v.z); }
    LR_API void ortho(m44f&, float l, float r, float b, float t, float n, float f);
    LR_API void frustum(m44f&, float l, float r, float b, float t, float n, float f);
    LR_API void perspective(m44f&, float fov, float aspect, float near, float far);
    LR_API void invert(m44f&);

    LR_API void decompose(m44f, v3f& t, v3f& r, v3f& s);
    LR_API void recompose(m44f& result, const v3f& t, const v3f& r, const v3f& s);
    LR_API void orthonormalize(m44f&);


	LR_CAPI m44f m44f_identity;

	template <typename T> inline float vector_length(const T & a) { return length(a); }

#ifdef HAVE_GLM
	template <typename T> inline T vector_normalize(const T & a) { return glm::normalize(a); }
#else
	template <typename T> inline T vector_normalize(const T & a) { return normalize(a); }
#endif

	template <typename T> inline float vector_dot(const T & a, const T & b) { return dot(a, b); }

	inline v3f vector_cross(const v3f & a, const v3f & b) { return cross(a, b); }
	LR_API v4f matrix_multiply(const m44f & a, const v4f & v);
	LR_API m44f matrix_multiply(const m44f & a, const m44f & b);

	LR_API m44f matrix_invert(const m44f & a);
	LR_API m44f matrix_transpose(const m44f & a);

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



    template <typename T>
    class Spinor
    {
    public:
        T i, j;

        Spinor()
        : i(1), j(0) { }

        Spinor(T angle)
        : i(std::cos(angle))
        , j(std::sin(angle)) { }

        Spinor(T i, T j)
        : i(i)
        , j(j) { }

        void set(T angle)
        {
            i = std::cos(angle);
            j = std::sin(angle);
        }

        void set(T i_, T j_)
        {
            i = i_;
            j = j_;
        }

        void scale(T s)
        {
            i *= s;
            j *= s;
        }

        void invert()
        {
            T s = i * i + j * j;
            i *= s;
            j *= -s;
        }

        T length() const
        {
            return std::sqrt(i * i + j * j);
        }

        T lengthSquared() const
        {
            return i * i + j * j;
        }

        Spinor operator+(const Spinor& rhs) const
        {
            return Spinor(i + rhs.i, j + rhs.j);
        }

        Spinor& operator+=(const Spinor& rhs)
        {
            i += rhs.i;
            j += rhs.j;
            return *this;
        }

        Spinor operator*(T rhs) const
        {
            return Spinor(i * rhs, j * rhs);
        }

        Spinor operator*(const Spinor& rhs) const
        {
            return Spinor(i * rhs.i - j * rhs.j, i * rhs.j + j * rhs.i);
        }

        Spinor& operator*=(T s)
        {
            i *= s;
            j *= s;
            return *this;
        }

        Spinor& operator*=(const Spinor& rhs)
        {
            T ti = rhs.i * i - rhs.j * j;
            j = rhs.j * i + rhs.i * j;
            i = ti;
            return *this;
        }

        Spinor& operator=(const Spinor& rhs)
        {
            i = rhs.i;
            j = rhs.j;
            return *this;
        }

        T dot(const Spinor& rhs) const
        {
            return i * rhs.i + j * rhs.j;
        }

        void normalize()
        {
            T oolength = T(1) / length();
            i *= oolength;
            j *= oolength;
        }

        T angle() const
        {
            return std::atan2(j, i);
        }
    };

    template<typename T>
    Spinor<T> lerp(const Spinor<T>& start, const Spinor<T>& end, T t)
    {
        Spinor<T> r(start);
        r.scale(T(1) - t);
        Spinor<T> s(end);
        s.scale(t);
        r += s;
        r.normalize();
        return r;
    }

    template<typename T>
    Spinor<T> slerp(const Spinor<T>& start, const Spinor<T>& end, T t)
    {
        // cosine
        T cosom = start.i * end.i + start.j * end.j;

        // adjust signs

        T ti, tj;
        if (cosom < 0) {
            cosom = -cosom;
            ti = -end.i;
            tj = -end.j;
        }
        else {
            ti = end.i;
            tj = end.j;
        }

        T scale0, scale1;
        // if close use linear interp
        if (T(1) - cosom > T(0.001)) {
            T omega = std::acos(cosom);
            T sinom = std::sin(omega);
            scale0 = std::sin((T(1) - t) * omega) / sinom;
            scale1 = std::sin(t * omega) / sinom;
        }
        else {
            scale0 = T(1) - t;
            scale1 = t;
        }

        Spinor<T> r(scale0 * start.i + scale1 * ti,
                scale0 * start.j + scale1 * tj);

        return r;
    }

    template<typename T>
    T slerp(T start, T end, T t)
    {
        Spinor<T> s0(std::cos(start / T(2)), std::sin(start / T(2)));
        Spinor<T> s1(std::cos(end / T(2)), std::sin(end / T(2)));
        return slerp<T>(s0, s1, t).angle();
    }


    /*
	    This function is based on equation 11 in

	    Rotating Objects Using Quaternions
	    Nick Bobick
	    Game Developer Magazine, Feb. 1998, pp. 34-42
     */

    inline quatf inputAngularVelocity(quatf result, float dt, const v3f& a)
    {
	    quatf velquat;
	    velquat.x = a.x;	// x
	    velquat.y = a.y;	// y
	    velquat.z = a.z;    // z
	    velquat.w = 0.f;    // w

        velquat = velquat * velquat * quatf(a.x, a.y, a.z, 1.0f) * (dt * 0.5f);
        result += velquat;
        return normalize(result);
    }

#ifdef HAVE_GLM
    inline quatf quatFromAxisAngle(const v3f& a, float rad)
    {
        return glm::angleAxis(rad, a);
    }

    inline v3f quatRotateVector(quatf q, v3f v)
    {
        return glm::rotate(q, v);
    }

#elif defined(HAVE_LINALG)
    inline quatf quatFromAxisAngle(const v3f& v, float a)
    {
		quatf Result;
        float s = std::sin(a * 0.5f);
		Result.w = std::cos(a * 0.5f);
		Result.x = v.x * s;
		Result.y = v.y * s;
		Result.z = v.z * s;
		return Result;
    }

    inline v3f quatRotateVector(quatf q, v3f v)
    {
        // https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
        v3f u {q.x, q.y, q.z};
        float s = q.w;

        return     2.f * dot(u, v)  * u
               +  (s*s - dot(u, u)) * v
               + 2.0f * s * cross(u, v);
    }
    
#endif

}

#define M44F(c1x, c1y, c1z, c1w, c2x, c2y, c2z, c2w, c3x, c3y, c3z, c3w, c4x, c4y, c4z, c4w) \
  { float(c1x), float(c1y), float(c1z), float(c1w), \
    float(c2x), float(c2y), float(c2z), float(c2w), \
    float(c3x), float(c3y), float(c3z), float(c3w), \
    float(c4x), float(c4y), float(c4z), float(c4w)  }
#define V2I(x, y) v2i(int32_t(x), int32_t(y))
#define V2F(x, y) v2f(float(x), float(y))
#define V3F(x, y, z) v3f(float(x), float(y), float(z))
#define V4F(x, y, z, w) v4f(float(x), float(y), float(z), float(w))
