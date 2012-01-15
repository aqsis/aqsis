// Aqsis
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
        STRONG_INLINE friend float4 operator+(float4 lhs, float4 rhs) { return float4 (_mm_add_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend float4 operator-(float4 lhs, float4 rhs) { return float4 (_mm_sub_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend float4 operator*(float4 lhs, float4 rhs) { return float4 (_mm_mul_ps (lhs.m_vec, rhs.m_vec)); }
        STRONG_INLINE friend float4 operator/(float4 lhs, float4 rhs) { return float4 (_mm_div_ps (lhs.m_vec, rhs.m_vec)); }

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

        // raw access to underlying data
        STRONG_INLINE __m128 get() const { return m_vec; }

        // load/store to float* loadu/storeu are the unaligned versions
        STRONG_INLINE static float4 load(const float* f) { return float4(_mm_load_ps(f)); }
        STRONG_INLINE static float4 loadu(const float* f) { return float4(_mm_loadu_ps(f)); }
        STRONG_INLINE static void store(float* f, float4 rhs) { _mm_store_ps(f, rhs.m_vec); }
        STRONG_INLINE static void storeu(float* f, float4 rhs) { _mm_storeu_ps(f, rhs.m_vec); }

        // indexing
        STRONG_INLINE float operator[](int i) const
        {
            float res[4];
            storeu(res, m_vec);
            return res[i];
        }

        // powers
        STRONG_INLINE friend float4 pow(float4 a, float4 b)
        {
            float af[4];
            float bf[4];
            storeu(af, a.m_vec);
            storeu(bf, b.m_vec);
            return float4(pow(af[0], bf[0]), pow(af[1], bf[1]),
                          pow(af[2], bf[2]), pow(af[3], bf[3]));
        }

        STRONG_INLINE friend float4 exp(float4 a)
        {
            float af[4];
            storeu(af, a.m_vec);
            return float4(exp(af[0]), exp(af[1]), exp(af[2]), exp(af[3]));
        }

        STRONG_INLINE friend float4 sqrt(float4 a)
        {
            return float4(_mm_sqrt_ps(a.m_vec));
        }

        STRONG_INLINE friend float4 abs(float4 a)
        {
            union {
                __m128 f;
                unsigned int u[4]; // int better be 32 bits wide
            } signMask = {{0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF}};
            return float4(_mm_and_ps(signMask.f, a.m_vec));
        }

        friend float4 min(float4 a, float4 b) { return float4(_mm_min_ps(a.m_vec, b.m_vec)); }
        friend float4 max(float4 a, float4 b) { return float4(_mm_max_ps(a.m_vec, b.m_vec)); }
};

std::ostream& operator<<(std::ostream& out, float4 f4)
{
    out << "(" << f4[0] << " " << f4[1] << " "
               << f4[2] << " " << f4[3] << ")";
    return out;
}

#endif // FLOAT4_H_INCLUDED
