// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

#ifndef AQSIS_UTIL_H_INCLUDED
#define AQSIS_UTIL_H_INCLUDED

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <vector>
#include <iostream>
#ifndef WIN32
#include <alloca.h>
#endif

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


namespace Aqsis {

typedef Imath::V3f Vec3;
typedef Imath::V2f Vec2;
typedef Imath::V2i V2i;
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

/// Free all memory held by a std::vector.
///
/// A std::vector implementation usually keeps the allocated memory around even
/// after the size decreases to zero (or perhaps even if reserve(0) is called).
/// Often this is what we want, but sometimes we really need to deallocate the
/// memory held by the vector entirely, hence this function.
template<typename T>
inline void vectorFree(std::vector<T>& v)
{
    std::vector<T> emptyVec;
    v.swap(emptyVec);
}

template<typename T>
inline Imath::Vec2<T> vec2_cast(const Imath::Vec3<T>& v)
{
    return Imath::Vec2<T>(v.x, v.y);
}

/// Product of vector components.
template<typename T>
inline T prod(const Imath::Vec2<T>& v)
{
    return v.x*v.y;
}

template<typename T>
inline Imath::Vec2<T> min(const Imath::Vec2<T>& a, const Imath::Vec2<T>& b)
{
    return Imath::Vec2<T>(std::min(a.x, b.x), std::min(a.y,b.y));
}

template<typename T>
inline Imath::Vec2<T> max(const Imath::Vec2<T>& a, const Imath::Vec2<T>& b)
{
    return Imath::Vec2<T>(std::max(a.x, b.x), std::max(a.y,b.y));
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
inline T maxNorm(Imath::Vec2<T> v)
{
    return std::max(std::fabs(v.x), std::fabs(v.y));
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

// Compute ceil(real(n)/d) using integers for positive n and d.
template<typename T>
inline T ceildiv(T n, T d)
{
    return (n-1)/d + 1;
}

inline float deg2rad(float d) { return (M_PI/180) * d; }
inline float rad2deg(float r) { return (180/M_PI) * r; }

#ifdef _WIN32
/// Calculate the base-2 logarithm of a number.
///
/// log2() as a C99 math function isn't avaliable on windows.
inline float log2(float x)
{
    // log2(x) = log(x)/log(2) ~= 1.4426950408889633 * log(x)
    return 1.4426950408889633 * std::log(x);
}
#endif

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

inline Mat4 orthographicProjection(float near, float far)
{
    float a = 2/(far-near);
    float b = -(far+near)/(far-near);
    return Mat4(1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, a, 0,
                0, 0, b, 1);
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

template<typename T>
inline Imath::V2i iceil(const Imath::Vec2<T>& v)
{
    return Imath::V2i(iceil(v.x), iceil(v.y));
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


} // namespace Aqsis
#endif // AQSIS_UTIL_H_INCLUDED
