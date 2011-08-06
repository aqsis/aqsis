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

#ifndef MICROQUAD_H_INCLUDED
#define MICROQUAD_H_INCLUDED

#include "util.h"
#include "sample.h"
#include <xmmintrin.h>

#include "float4.h"

struct V4
{
    const float* f;
    V4(const float* f) : f(f) {}
};

inline std::ostream& operator<<(std::ostream& out, V4 f)
{
    out << f.f[0] << "," << f.f[1] << "," << f.f[2] << "," << f.f[3];
}

inline std::ostream& operator<<(std::ostream& out, __m128 f128)
{
    float f[4] __attribute__((aligned(16)));
    _mm_store_ps(f, f128);
    out << V4(f);
}

class PointInQuadSSE
{
    private:
        // Hit test coefficients
        float m_xmul[4];
        float m_ymul[4];
        float m_offset[4];

        __m128 m_xmul_sse;
        __m128 m_ymul_sse;
        __m128 m_offset_sse;

        inline void setupEdgeEq(int i, const Vec2& a, const Vec2& b)
        {
            Vec2 v = b - a;
            m_offset[i] = cross(v, a);
            m_xmul[i] = v.y;
            m_ymul[i] = -v.x;
        }

    public:
        // Cyclic vertex order:
        // a -- b
        // |    |
        // d -- c
        PointInQuadSSE(Vec3 a, Vec3 b, Vec3 c, Vec3 d)
        {
            // (1) Orientation test
            // (2) Convexity test
            // (3) Compute coefficients
            setupEdgeEq(0, vec2_cast(a), vec2_cast(b));
            setupEdgeEq(1, vec2_cast(b), vec2_cast(c));
            setupEdgeEq(2, vec2_cast(c), vec2_cast(d));
            setupEdgeEq(3, vec2_cast(d), vec2_cast(a));

            // Unaligned SSE loads
            m_xmul_sse = _mm_loadu_ps(m_xmul);
            m_ymul_sse = _mm_loadu_ps(m_ymul);
            m_offset_sse = _mm_loadu_ps(m_offset);

            float out[4] __attribute__((aligned(16)));
            _mm_store_ps(out, m_offset_sse);
        }

        // point-in-polygon test
        bool operator()(const Sample& samp)
        {
            __m128 x = _mm_set1_ps(samp.p.x);
            __m128 y = _mm_set1_ps(samp.p.y);
            __m128 edge = m_xmul_sse*x + m_ymul_sse*y + m_offset_sse;
            __m128 cmp = _mm_cmplt_ps(edge, _mm_setzero_ps());
            union {
                float f[4];
                int i[4];
            } out __attribute__((aligned(16)));
            _mm_store_ps(out.f, cmp);
            return out.i[0] && out.i[1] && out.i[2] && out.i[3];
//            return out.i[0];
        }
};

class PointsInQuad
{
    private:
        // Hit test coefficients
        __m128 m_xmul[4];
        __m128 m_ymul[4];
        __m128 m_offset[4];

    public:
        inline void setupEdgeEq(int i, const Vec2& a, const Vec2& b)
        {
            Vec2 v = b - a;
            m_offset[i] = _mm_set1_ps(-cross(v, a));
            m_xmul[i] = _mm_set1_ps(v.y);
            m_ymul[i] = _mm_set1_ps(-v.x);
        }

    public:
        PointsInQuad(Vec3 a, Vec3 b, Vec3 c, Vec3 d)
        {
            setupEdgeEq(0, vec2_cast(a), vec2_cast(b));
            setupEdgeEq(1, vec2_cast(b), vec2_cast(c));
            setupEdgeEq(2, vec2_cast(c), vec2_cast(d));
            setupEdgeEq(3, vec2_cast(d), vec2_cast(a));
        }

        __m128 contains(__m128 px, __m128 py) const
        {
            return _mm_and_ps( _mm_and_ps( _mm_and_ps(
                _mm_cmple_ps(m_xmul[0]*px + m_ymul[0]*py, m_offset[0]),
                _mm_cmple_ps(m_xmul[1]*px + m_ymul[1]*py, m_offset[1])),
                _mm_cmplt_ps(m_xmul[2]*px + m_ymul[2]*py, m_offset[2])),
                _mm_cmplt_ps(m_xmul[3]*px + m_ymul[3]*py, m_offset[3]));
        }

        STRONG_INLINE int contains(float4 px, float4 py) const
        {
//            return 
//                (m_xmul[0]*px + m_ymul[0]*py <= m_offset[0])
//                & (m_xmul[1]*px + m_ymul[1]*py <= m_offset[1])
//                & (m_xmul[2]*px + m_ymul[2]*py <  m_offset[2])
//                & (m_xmul[3]*px + m_ymul[3]*py <  m_offset[3]);
            return 
                ( (m_xmul[0]*px + m_ymul[0]*py <= m_offset[0])
                & (m_xmul[1]*px + m_ymul[1]*py <= m_offset[1])
                & (m_xmul[2]*px + m_ymul[2]*py <  m_offset[2])
                & (m_xmul[3]*px + m_ymul[3]*py <  m_offset[3])
                ).to_mask();
        }
};

class PointInQuad
{
    private:
        // Hit test coefficients
        float m_xmul[4];
        float m_ymul[4];
        float m_offset[4];

        inline void setupEdgeEq(int i, const Vec2& a, const Vec2& b)
        {
            Vec2 v = b - a;
            m_offset[i] = cross(v, a);
            m_xmul[i] = v.y;
            m_ymul[i] = -v.x;
        }

    public:
        // Cyclic vertex order:
        // a -- b
        // |    |
        // d -- c
        PointInQuad(Vec3 a, Vec3 b, Vec3 c, Vec3 d)
        {
            // (1) Orientation test
            // (2) Convexity test
            // (3) Compute coefficients
            setupEdgeEq(0, vec2_cast(a), vec2_cast(b));
            setupEdgeEq(1, vec2_cast(b), vec2_cast(c));
            setupEdgeEq(2, vec2_cast(c), vec2_cast(d));
            setupEdgeEq(3, vec2_cast(d), vec2_cast(a));
        }

        // point-in-polygon test
        bool operator()(const Sample& samp)
        {
            float x = samp.p.x;
            float y = samp.p.y;
            return  (m_xmul[0]*x + m_ymul[0]*y + m_offset[0] <= 0)
                 && (m_xmul[1]*x + m_ymul[1]*y + m_offset[1] <= 0)
                 && (m_xmul[2]*x + m_ymul[2]*y + m_offset[2] <  0)
                 && (m_xmul[3]*x + m_ymul[3]*y + m_offset[3] <  0);
        }
};


//class PointInQuads
//{
//        inline void setupEdgeEq(int i, __m128 ax, __m128 ay, __m128 bx, __m128 by)
//        {
//            __m128 vx = bx - ax;
//            __m128 vy = by - ay;
//            m_offset[i] = -(vx*ay - ax*vy);
//            m_xmul[i] = vy;
//            m_ymul[i] = vx;
//        }
//    public:
//        PointInQuads(Vec3 a[4], Vec3 b[4], Vec3 c[4], Vec3 d[4])
//        {
//#define LOAD_COMPS(v, comp) _mm_set_ps(v[0].comp, v[1].comp, v[2].comp, v[3].comp)
//#define LOAD_COMP_XY(v) LOAD_COMPS(v, x), LOAD_COMPS(v, y)
//            setupEdgeEq(0, LOAD_COMP_XY(a), LOAD_COMP_XY(b));
//            setupEdgeEq(1, LOAD_COMP_XY(b), LOAD_COMP_XY(c));
//            setupEdgeEq(2, LOAD_COMP_XY(c), LOAD_COMP_XY(d));
//            setupEdgeEq(3, LOAD_COMP_XY(d), LOAD_COMP_XY(a));
//        }


#endif // MICROQUAD_H_INCLUDED
