// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <cassert>
#include <cfloat>
#include <cmath>
#include <vector>
#include <iostream>
#ifndef WIN32
#include <alloca.h>
#endif

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/arithmetic_traits.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/intrusive_ptr.hpp>

// The distinction here is only due to the layout of the 
// win32libs copy of OpenEXR.
// Ideally, we should modify the win32libs structure to 
// properly match OpenEXR and drop this.
#ifndef WIN32
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathColor.h>
#else
#include <ImathBox.h>
#include <ImathMatrix.h>
#include <ImathVec.h>
#include <ImathColor.h>
#endif


typedef Imath::V3f Vec3;
typedef Imath::V2f Vec2;
typedef Imath::M44f Mat4;
typedef Imath::M33f Mat3;
typedef Imath::C3f Col3;

typedef Imath::Box3f Box;


inline std::ostream& operator<<(std::ostream& out, Box b)
{
    out << "[" << b.min << " -- " << b.max << "]";
    return out;
}

template<typename T>
inline T* get(std::vector<T>& v)
{
    assert(v.size() > 0);
    return &v[0];
}
template<typename T>
inline const T* get(const std::vector<T>& v)
{
    assert(v.size() > 0);
    return &v[0];
}


template<typename T>
inline Imath::Vec2<T> vec2_cast(const Imath::Vec3<T>& v)
{
    return Imath::Vec2<T>(v.x, v.y);
}

template<typename T>
inline T cross(Imath::Vec2<T> a, Imath::Vec2<T> b)
{
    return a.x*b.y - b.x*a.y;
}

inline float dot(Vec3 a, Vec3 b)
{
    return a.dot(b);
}


template<typename T>
inline typename boost::enable_if<boost::is_arithmetic<T>, T>::type
max(const T a, const T b)
{
    return (a < b) ? b : a;
}

template<typename T>
inline T maxNorm(Imath::Vec2<T> v)
{
    return max(std::fabs(v.x), std::fabs(v.y));
}

/** \brief Bilinear interpolation.
 *
 * Bilinear interpolation over a "quadrilateral" of values arranged in the
 * standard order as follows:
 *
 * \verbatim
 *
 *   c---d    ^
 *   |   |    | v-direction [ uv.y() ]
 *   a---b    |
 *
 *   u-direction -->
 *   [ uv.x() ]
 *
 * \endverbatim
 *
 * This performs no checks for whether u and v are between 0 and 1 and may give
 * odd results if they're not.
 *
 * \param a,b,c,d - quadrilateral corners.
 * \param uv - Parametric (u,v) coordinates, should be in the interval [0,1]
 */
template<typename T>
inline T bilerp(T a, T b, T c, T d, float u, float v)
{
    float w0 = (1-v)*(1-u);
    float w1 = (1-v)*u;
    float w2 = v*(1-u);
    float w3 = v*u;
    return w0*a + w1*b + w2*c + w3*d;
}
template<typename T>
inline T bilerp(T a, T b, T c, T d, Vec2 uv)
{
    return bilerp(a,b,c,d, uv.x, uv.y);
}

template<typename T>
inline T lerp(T a, T b, float t)
{
    return (1-t)*a + t*b;
}

template<typename T>
inline T clamp(T x, T low, T high)
{
    return (x < low) ? low : ((x > high) ? high : x);
}

template<typename T>
inline int ifloor(T x)
{
    int ix = static_cast<int>(x);
    if(x >= 0)
        return ix;
    else
        return ix - (x != ix);
}

template<typename T>
inline int iceil(T x)
{
    int ix = static_cast<int>(x);
    if(x <= 0)
        return ix;
    else
        return ix + (x != ix);
}

inline float deg2rad(float d) { return (M_PI/180) * d; }
inline float rad2deg(float r) { return (180/M_PI) * r; }

inline Mat4 perspectiveProjection(float fov, float near, float far)
{
    float s = 1/std::tan(deg2rad(fov)/2);
    float a = far/(far-near);
    float b = -near*a;
    return Mat4(s, 0, 0, 0,
                0, s, 0, 0,
                0, 0, a, 1,
                0, 0, b, 0);
}

inline Mat4 screenWindow(float left, float right, float bottom, float top)
{
    float w = right-left;
    float h = top-bottom;
    return Mat4(2/w, 0, 0, 0,
                0, 2/h, 0, 0,
                0, 0,   1, 0,
                -(right+left)/w, -(top+bottom)/h, 0, 1);
}

/// Get the vector transformation associated with the point transformation, m
inline Mat3 vectorTransform(const Mat4& m)
{
    return Mat3(m[0][0], m[0][1], m[0][2],
                m[1][0], m[1][1], m[1][2],
                m[2][0], m[2][1], m[2][2]);
}

/// Get the normal transformation associated with the point transformation, m
inline Mat3 normalTransform(const Mat4& m)
{
    Mat3 nTrans = vectorTransform(m);
    nTrans.invert();
    nTrans.transpose();
    return nTrans;
}


/// Transform a bounding box
inline Box transformBound(const Box& bound, const Mat4& m)
{
    Vec3 v1 = bound.min;
    Vec3 v2 = bound.max;
    Box b(v1*m);
    b.extendBy(Vec3(v2.x, v1.y, v1.z)*m);
    b.extendBy(Vec3(v1.x, v2.y, v1.z)*m);
    b.extendBy(Vec3(v1.x, v1.y, v2.z)*m);
    b.extendBy(Vec3(v1.x, v2.y, v2.z)*m);
    b.extendBy(Vec3(v2.x, v1.y, v2.z)*m);
    b.extendBy(Vec3(v2.x, v2.y, v1.z)*m);
    b.extendBy(v2*m);
    return b;
}

/// Transform v into the "hybrid camera / raster" coordinate system
///
/// Hybrid coordinates are given by projecting the x and y coordinates of v,
/// but leaving the z coordinate untouched.
inline Vec3 hybridRasterTransform(const Vec3& v, const Mat4& m)
{
    float x = v.x*m[0][0] + v.y*m[1][0] + v.z*m[2][0] + m[3][0];
    float y = v.x*m[0][1] + v.y*m[1][1] + v.z*m[2][1] + m[3][1];
    float w = v.x*m[0][3] + v.y*m[1][3] + v.z*m[2][3] + m[3][3];
    float invW = 1/w;

    return Vec3(x*invW, y*invW, v.z);
}

template<typename T>
inline Imath::V2i ifloor(const Imath::Vec2<T>& v)
{
    return Imath::V2i(ifloor(v.x), ifloor(v.y));
}

#define ALLOCA(type, len) static_cast<type*>(alloca(len*sizeof(type)))
#define FALLOCA(len) ALLOCA(float, len)

template<typename T, size_t sz> int array_len(T (&a)[sz]) { return sz; }


/// Radical inverse function for low-discrepancy sequences.
///
/// The radical inverse of n in base b is the base b digits of n reversed and
/// placed to the right of the radix point:
///
///     n = d1d2d3d4d5 |->  0.d5d4d3d2d1 = radicalInverse(n)
///
/// These are great for easily building sequences in D dimensions with nice
/// distribution properties; just construct tuples like
///
/// [radicalInverse(n, 2), radicalInverse(n, 3), ..., radicalInverse(n, p_D)]
///
/// where p_D is the D'th prime number.  Note that p_D can't be too large, or
/// the sequences start looking rather non-uniform.
inline float radicalInverse(int n, int base = 2)
{
    double r = 0;
    double invBase = 1.0/base;
    double digitMult = invBase;
    while(n != 0)
    {
        r += digitMult*(n % base);
        n /= base;
        digitMult *= invBase;
    }
    return r;
}


//------------------------------------------------------------------------------
/// Reference counting machinary.

inline void nullDeleter(const void*) { }

/// Reference counted base mixin for use with boost::intrusive_ptr.
///
/// This is a non-virtual implementation for maximum efficiency.
class RefCounted
{
    public:
        RefCounted() : m_refCount(0) {}

        /// Copying does *not* copy the reference count!
        RefCounted(const RefCounted& /*r*/) : m_refCount(0) {}
        RefCounted& operator=(const RefCounted& /*r*/) { return *this; }

        int useCount() const { return m_refCount; }
        int incRef() const   { return ++m_refCount; }
        int decRef() const   { return --m_refCount; }

    protected:
        /// Protected so users can't delete RefCounted directly.
        ~RefCounted() {}

    private:
        mutable int m_refCount;
};


/// Add a reference to a RefCounted object.
inline void intrusive_ptr_add_ref(RefCounted* p)
{
    p->incRef();
}

/// Release a reference to a RefCounted object.
///
/// Note that this function *must* be a template, because RefCounted does not
/// have a virtual destructor.  (Therefore, if we just took p as type
/// RefCounted*, the wrong destructor would get called!)
template<typename T>
inline typename boost::enable_if<boost::is_base_of<RefCounted, T> >::type
intrusive_ptr_release(T* p)
{
    if(p->decRef() == 0)
        delete p;
}


#endif // UTIL_H_INCLUDED
