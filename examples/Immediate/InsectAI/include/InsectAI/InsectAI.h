
// Copyright (c) 2003-2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

/** @file	InsectAI.h
	@brief	The main public interface to the InsectAI system
*/

#pragma once

typedef float Real;
typedef unsigned int uint32;

#include <array>
#include <stdlib.h>

namespace Lab
{
    struct DynamicState;

    typedef std::array<float, 3> LabVec3;


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


}

/// @namespace	InsectAI
/// @brief		The main public interface is contained in the InsectAI namespace
namespace InsectAI
{

	class Entity;
	class EntityDatabase
    {
	public:
		EntityDatabase() { }
		virtual ~EntityDatabase() { }

        virtual Lab::DynamicState* GetNearest(Entity*, uint32 filter) = 0;
	};


    /// return a random number between zero and one
    inline Real randf()
    {
        Real retval = (Real) (rand() & 0x7fff);
        return retval * (1.0f / 32768.0f);
    }

    /// return a random number within the specified range (inclusive)
    inline Real randf(Real minR, Real maxR)
    {
        Real retval = randf() * (maxR - minR);
        return retval + minR;
    }

}	// end namespace InsectAI

