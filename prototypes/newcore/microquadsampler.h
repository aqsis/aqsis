#ifndef QUADRASTERIZER_H_INCLUDED
#define QUADRASTERIZER_H_INCLUDED

#include "invbilin.h"
#include "options.h"
#include "sample.h"
#include "util.h"
#include "pointinquad.h"


/// Quadrilateral micropolygon sampler
///
/// This is designed to be constructed just before sampling time; it's not
/// memory efficient, and should not be a long-lived data structure.
class MicroQuadSampler
{
    private:
        // Grid being sampled.
        const QuadGrid& m_grid;
        // Iterator for the current micropoly
        QuadGrid::Iterator m_curr;

        // Grid indices for vertices
        MicroQuadInd m_ind;
        // Point-in-polygon tests
        PointInQuad m_hitTest;
        // Storage for the micropoly data
        ConstDataView<Vec3> m_P;

        // Shading interpolation
        InvBilin m_invBilin;
        // uv coordinates of current interpolation point
        Vec2 m_uv;
        // Whether to use smooth shading or not.
        bool m_smoothShading;

    public:
        // Cyclic vertex order:
        // a -- b
        // |    |
        // d -- c
        MicroQuadSampler(const QuadGrid& grid, const Options& opts)
            : m_grid(grid),
            m_curr(grid.begin()),
            m_ind(*m_curr),
            m_hitTest(),
            m_P(grid.storage().P()),
            m_uv(0.0f),
            m_smoothShading(opts.smoothShading)
        { }

        /// Advance to next micropolygon.
        void next()
        {
            ++m_curr;
            m_ind = *m_curr;
        }

        bool valid()
        {
            return m_curr.valid();
        }

        Box bound() const
        {
            Box bnd(m_P[m_ind.a]);
            bnd.extendBy(m_P[m_ind.b]);
            bnd.extendBy(m_P[m_ind.c]);
            bnd.extendBy(m_P[m_ind.d]);
            return bnd;
        }

        // Initialize the hit test
        inline void initHitTest()
        {
            m_hitTest.init(vec2_cast(m_P[m_ind.a]), vec2_cast(m_P[m_ind.b]),
                           vec2_cast(m_P[m_ind.c]), vec2_cast(m_P[m_ind.d]),
                           (m_curr.u() + m_curr.v()) % 2);
        }
        // Returns true if the sample is contained in the polygon
        inline bool contains(const Sample& samp)
        {
            return m_hitTest(samp);
        }

        // Initialize the shading interpolator
        inline void initInterpolator()
        {
            if(m_smoothShading)
            {
                m_invBilin.init(
                    vec2_cast(m_P[m_ind.a]), vec2_cast(m_P[m_ind.b]),
                    vec2_cast(m_P[m_ind.d]), vec2_cast(m_P[m_ind.c]) );
            }
        }

        inline void interpolateAt(const Sample& samp)
        {
            if(m_smoothShading)
                m_uv = m_invBilin(samp.p);
        }

        inline float interpolateZ() const
        {
            if(m_smoothShading)
                return bilerp(m_P[m_ind.a].z, m_P[m_ind.b].z,
                              m_P[m_ind.d].z, m_P[m_ind.c].z, m_uv);
            else
                return m_P[m_ind.a].z;
        }

        inline void interpolateColor(float* col) const
        {
            int CsIdx = m_grid.storage().varList().stdIndices().Cs;
            assert(CsIdx >= 0);
            ConstFvecView Cs = m_grid.storage().get(CsIdx);
            if(m_smoothShading)
            {
                for(int i = 0; i < 3; ++i)
                    col[i] = bilerp(Cs[m_ind.a][i], Cs[m_ind.b][i],
                                    Cs[m_ind.d][i], Cs[m_ind.c][i], m_uv);
            }
            else
            {
                for(int i = 0; i < 3; ++i)
                    col[i] = Cs[m_ind.a][i];
            }
        }
};


#endif // QUADRASTERIZER_H_INCLUDED
