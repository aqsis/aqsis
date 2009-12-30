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
        // Grid indices for vertices
        MicroQuadInd m_ind;
        // Point-in-polygon tests
        PointInQuad m_hitTest;
        // Storage for the micropoly data
        const GridvarStorage& m_storage;
        ConstDataView<Vec3> m_P;

        // Shading interpolation
        InvBilin m_invBilin;
        // uv coordinates of current interpolation point
        Vec2 m_uv;
        // Whether to use smooth shading or not.
        bool m_smoothShading;

        // Which point-on-edge to use in edge tests
        bool m_flipEnd;

    public:
        // Cyclic vertex order:
        // a -- b
        // |    |
        // d -- c
        MicroQuadSampler(const QuadGrid::Iterator& i)
            : m_ind(*i),
            m_hitTest(),
            m_storage(i.grid().storage()),
            m_P(m_storage.P()),
            m_flipEnd((i.u() + i.v()) % 2 )
        { }

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
                           m_flipEnd);
        }
        // Returns true if the sample is contained in the polygon
        inline bool contains(const Sample& samp)
        {
            return m_hitTest(samp);
        }

        // Initialize the shading interpolator
        inline void initInterpolator(const Options& opts)
        {
            m_smoothShading = opts.smoothShading;
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
            int CsIdx = m_storage.varList().stdIndices().Cs;
            assert(CsIdx >= 0);
            ConstFvecView Cs = m_storage.get(CsIdx);
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


//class MicroQuadSampler
//{
//    private:
//        const QuadGrid& m_grid;
//
//    public:
//        MicroQuadSampler(const QuadGrid& grid)
//            : m_grid(grid)
//        { }
//
//        /// Advance to the next quad in the grid.
//        void nextQuad()
//        {
//        }
//};



#endif // QUADRASTERIZER_H_INCLUDED
