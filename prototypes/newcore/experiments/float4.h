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

#ifndef FLOAT4_H_INCLUDED
#define FLOAT4_H_INCLUDED

#include <xmmintrin.h>

#include <iostream>

#define STRONG_INLINE __attribute__((always_inline))

class bool4
{
    private:
        __m128 m_vec;

        friend class float4;

    public:
        STRONG_INLINE bool4() { /*uninitialized*/ }
        STRONG_INLINE bool4(bool b) { m_vec = (b ? _mm_set1_ps(0xFFFFFFFF) : _mm_setzero_ps()); }
        STRONG_INLINE bool4(__m128 b4) { m_vec = b4; }

        // bool4 logical operators
        STRONG_INLINE friend bool4 operator&(bool4 lhs, bool4 rhs) { return bool4 (_mm_and_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend bool4 operator|(bool4 lhs, bool4 rhs) { return bool4 (_mm_or_ps  (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend bool4 operator^(bool4 lhs, bool4 rhs) { return bool4 (_mm_xor_ps (lhs.m_vec, rhs.m_vec)); }

        // get integer mask containing four packed bits
        STRONG_INLINE int to_mask() const { return _mm_movemask_ps(m_vec); }

        STRONG_INLINE __m128 get() const { return m_vec; }
};

class float4
{
    private:
        __m128 m_vec;

    public:
        // constructors from various formats
        STRONG_INLINE float4() { /*uninitialized*/ }
        STRONG_INLINE float4(float a) { m_vec = _mm_set1_ps(a); }
        STRONG_INLINE float4(__m128 f4) { m_vec = f4; }
        STRONG_INLINE float4(float a, float b, float c, float d) { m_vec = _mm_setr_ps(a,b,c,d); }

        // float4, float4 arithmetic
        STRONG_INLINE friend float4 operator+(float4 rhs, float4 lhs) { return float4 (_mm_add_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend float4 operator-(float4 rhs, float4 lhs) { return float4 (_mm_sub_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend float4 operator*(float4 rhs, float4 lhs) { return float4 (_mm_mul_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend float4 operator/(float4 rhs, float4 lhs) { return float4 (_mm_div_ps (lhs.m_vec, rhs.m_vec)); }

        // float4, float arithmetic
        STRONG_INLINE friend float4 operator+(float4 lhs, float rhs) { return float4 (_mm_add_ps (lhs.m_vec, _mm_set1_ps(rhs))); }
        STRONG_INLINE friend float4 operator-(float4 lhs, float rhs) { return float4 (_mm_sub_ps (lhs.m_vec, _mm_set1_ps(rhs))); }
        STRONG_INLINE friend float4 operator*(float4 lhs, float rhs) { return float4 (_mm_mul_ps (lhs.m_vec, _mm_set1_ps(rhs))); }
        STRONG_INLINE friend float4 operator/(float4 lhs, float rhs) { return float4 (_mm_div_ps (lhs.m_vec, _mm_set1_ps(rhs))); }
        STRONG_INLINE friend float4 operator+(float lhs, float4 rhs) { return float4 (_mm_add_ps (_mm_set1_ps(lhs), rhs.m_vec)); }
        STRONG_INLINE friend float4 operator-(float lhs, float4 rhs) { return float4 (_mm_sub_ps (_mm_set1_ps(lhs), rhs.m_vec)); }
        STRONG_INLINE friend float4 operator*(float lhs, float4 rhs) { return float4 (_mm_mul_ps (_mm_set1_ps(lhs), rhs.m_vec)); }
        STRONG_INLINE friend float4 operator/(float lhs, float4 rhs) { return float4 (_mm_div_ps (_mm_set1_ps(lhs), rhs.m_vec)); }

        // float4 comparison operators
        STRONG_INLINE friend bool4 operator< (float4 lhs, float4 rhs) { return bool4 (_mm_cmplt_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend bool4 operator<=(float4 lhs, float4 rhs) { return bool4 (_mm_cmple_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend bool4 operator> (float4 lhs, float4 rhs) { return bool4 (_mm_cmpgt_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend bool4 operator>=(float4 lhs, float4 rhs) { return bool4 (_mm_cmpge_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend bool4 operator==(float4 lhs, float4 rhs) { return bool4 (_mm_cmpeq_ps (lhs.m_vec, rhs.m_vec)); }

        // TODO: swizzle operations

        // assignment and opassign functions
        STRONG_INLINE float4& operator=(float4 rhs) { m_vec = rhs.m_vec; return *this; }
        STRONG_INLINE float4& operator+=(float4 rhs) { m_vec = m_vec + rhs.m_vec; return *this; }
        STRONG_INLINE float4& operator-=(float4 rhs) { m_vec = m_vec - rhs.m_vec; return *this; }
        STRONG_INLINE float4& operator*=(float4 rhs) { m_vec = m_vec * rhs.m_vec; return *this; }
        STRONG_INLINE float4& operator/=(float4 rhs) { m_vec = m_vec / rhs.m_vec; return *this; }

        // indexing
        STRONG_INLINE float operator[](int i) const { return reinterpret_cast<const float*>(&m_vec)[i]; }

        // raw access to underlying data
        STRONG_INLINE __m128 get() const { return m_vec; }

        // load/store to float* loadu/storeu are the unaligned versions
        STRONG_INLINE static float4 load(const float* f) { return float4(_mm_load_ps(f)); }
        STRONG_INLINE static float4 loadu(const float* f) { return float4(_mm_loadu_ps(f)); }
        STRONG_INLINE static void store(float* f, float4 rhs) { _mm_store_ps(f, rhs.m_vec); }
        STRONG_INLINE static void storeu(float* f, float4 rhs) { _mm_storeu_ps(f, rhs.m_vec); }
};

std::ostream& operator<<(std::ostream& out, float4 f4)
{
    out << "(" << f4[0] << " " << f4[1] << " "
               << f4[2] << " " << f4[3] << ")";
    return out;
}

#endif // FLOAT4_H_INCLUDED
