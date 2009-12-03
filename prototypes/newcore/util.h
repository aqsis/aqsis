#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <assert.h>
#include <cfloat>
#include <cmath>
#include <vector>
#include <iostream>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/arithmetic_traits.hpp>

#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathVec.h>

template<typename T>
T* get(std::vector<T>& v) { return v.empty() ? 0 : v[0]; }

template<typename T>
const T* get(const std::vector<T>& v) { return v.empty() ? 0 : v[0]; }


typedef Imath::V3f Vec3;
typedef Imath::V2f Vec2;
typedef Imath::M44f Mat4;

typedef Imath::Box3f Box;


std::ostream& operator<<(std::ostream& out, Box b)
{
    out << "[" << b.min << " -- " << b.max << "]";
    return out;
}


template<typename T>
Imath::Vec2<T> vec2_cast(const Imath::Vec3<T>& v)
{
    return Imath::Vec2<T>(v.x, v.y);
}

template<typename T>
T cross(Imath::Vec2<T> a, Imath::Vec2<T> b)
{
    return a.x*b.y - b.x*a.y;
}

template<typename T>
inline typename boost::enable_if<boost::is_arithmetic<T>, T>::type
max(const T a, const T b)
{
    return (a < b) ? b : a;
}

template<typename T>
T maxNorm(Imath::Vec2<T> v)
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
inline T bilerp(T a, T b, T c, T d, Vec2 uv)
{
    float w0 = (1-uv.y)*(1-uv.x);
    float w1 = (1-uv.y)*uv.x;
    float w2 = uv.y*(1-uv.x);
    float w3 = uv.y*uv.x;
    return w0*a + w1*b + w2*c + w3*d;
}

#endif // UTIL_H_INCLUDED
