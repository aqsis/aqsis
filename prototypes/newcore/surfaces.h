#ifndef SURFACES_H_INCLUDED
#define SURFACES_H_INCLUDED

#include "geometry.h"
#include "options.h"
#include "util.h"

#include "primvar.h"
#include "gridvar.h"


class Patch : public Geometry
{
    private:
        const Options& m_opts;
        boost::shared_ptr<PrimvarStorage> m_vars;

        // uv coordinates for corners of the base patch.
        const float m_uMin, m_uMax;
        const float m_vMin, m_vMax;

        void dice(RenderQueue& queue, int nu, int nv) const
        {
            boost::shared_ptr<GridvarList> gvarList(
                    new GridvarList(m_vars->varList()) );
            boost::shared_ptr<QuadGrid> grid(new QuadGrid(nu, nv, gvarList));

            // Create some space to store the variable temporaries.
            int maxAgg = gvarList->maxAggregateSize();
            float* aMin = FALLOCA(maxAgg);
            float* aMax = FALLOCA(maxAgg);

            float du = (m_uMax-m_uMin)/(nu-1);
            float dv = (m_vMax-m_vMin)/(nv-1);

            for(int ivar = 0, nvars = m_vars->varList().size();
                ivar < nvars; ++ivar)
            {
                ConstFvecView pvar = m_vars->get(ivar);
                FvecView gvar = grid->storage().get(ivar);
                int size = gvar.size();

                if((*gvarList)[ivar].uniform)
                {
                    // Uniform, no interpolation.
                    float* out = gvar[0];
                    const float* in = pvar[0];
                    for(int i = 0; i < size; ++i)
                        out[i] = in[i];
                }
                else
                {
                    // linear interpolation for Varying, Vertex, FaceVarying,
                    // FaceVertex.
                    const float* a1 = pvar[0];
                    const float* a2 = pvar[1];
                    const float* a3 = pvar[2];
                    const float* a4 = pvar[3];
                    for(int v = 0; v < nv; ++v)
                    {
                        float fv = m_vMin + dv*v;
                        // Get endpoints of current segment via linear
                        // interpolation
                        for(int i = 0; i < size; ++i)
                        {
                            aMin[i] = lerp(a1[i], a3[i], fv);
                            aMax[i] = lerp(a2[i], a4[i], fv);
                        }
                        // Interpolate between endpoints
                        for(int u = 0; u < nu; ++u)
                        {
                            float fu = m_uMin + du*u;
                            float* out = gvar[u];
                            for(int i = 0; i < size; ++i)
                                out[i] = lerp(aMin[i], aMax[i], fu);
                        }
                        gvar += nu;
                    }
                }
            }
            queue.push(grid);
        }

        void getCorners(Vec3& a, Vec3& b, Vec3& c, Vec3& d) const
        {
            ConstDataView<Vec3> P = m_vars->P();
            a = bilerp(P[0], P[1], P[2], P[3], m_uMin, m_vMin);
            b = bilerp(P[0], P[1], P[2], P[3], m_uMax, m_vMin);
            c = bilerp(P[0], P[1], P[2], P[3], m_uMin, m_vMax);
            d = bilerp(P[0], P[1], P[2], P[3], m_uMax, m_vMax);
        }

        Patch(const Options& opts, boost::shared_ptr<PrimvarStorage> vars,
                float uMin, float uMax, float vMin, float vMax)
            : m_opts(opts),
            m_vars(vars),
            m_uMin(uMin), m_uMax(uMax),
            m_vMin(vMin), m_vMax(vMax)
        { }

    public:
        Patch(const Options& opts, boost::shared_ptr<PrimvarStorage> vars)
            : m_opts(opts),
            m_vars(vars),
            m_uMin(0), m_uMax(1),
            m_vMin(0), m_vMax(1)
        { }

        virtual void splitdice(const Mat4& splitTrans, RenderQueue& queue) const
        {
            Vec3 a,b,c,d;
            getCorners(a,b,c,d);

            // Transform points into "splitting coordinates"
            Vec3 aSpl = a * splitTrans;
            Vec3 bSpl = b * splitTrans;
            Vec3 cSpl = c * splitTrans;
            Vec3 dSpl = d * splitTrans;

            // Diceable test: Estimate area as the sum of the areas of two
            // triangles which make up the patch.
            float area = 0.5 * (  ((bSpl-aSpl)%(cSpl-aSpl)).length()
                                + ((bSpl-dSpl)%(cSpl-dSpl)).length() );

            const float maxArea = m_opts.gridSize*m_opts.gridSize
                                  * m_opts.shadingRate;

            // estimate length in a-b, c-d direction
            float lu = 0.5*((bSpl-aSpl).length() + (dSpl-cSpl).length());
            // estimate length in a-c, b-d direction
            float lv = 0.5*((cSpl-aSpl).length() + (dSpl-bSpl).length());

            if(area <= maxArea)
            {
                // When the area (in number of micropolys) is small enough,
                // dice the surface.
                int nu = 1 + Imath::floor(lu/std::sqrt(m_opts.shadingRate));
                int nv = 1 + Imath::floor(lv/std::sqrt(m_opts.shadingRate));
                dice(queue, nu, nv);
            }
            else
            {
                // Otherwise, split the surface.  The splitting direction is
                // the shortest edge.

                // Split
                if(lu > lv)
                {
                    // split in the middle of the a-b and c-d sides.
                    // a---b
                    // | | |
                    // c---d
                    float uMid = 0.5*(m_uMin + m_uMax);
                    queue.push(boost::shared_ptr<Geometry>(
                                new Patch(m_opts, m_vars, m_uMin, uMid, m_vMin, m_vMax)));
                    queue.push(boost::shared_ptr<Geometry>(
                                new Patch(m_opts, m_vars, uMid, m_uMax, m_vMin, m_vMax)));
                }
                else
                {
                    // split in the middle of the a-c and b-d sides.
                    // a---b
                    // |---|
                    // c---d
                    float vMid = 0.5*(m_vMin + m_vMax);
                    queue.push(boost::shared_ptr<Geometry>(
                                new Patch(m_opts, m_vars, m_uMin, m_uMax, m_vMin, vMid)));
                    queue.push(boost::shared_ptr<Geometry>(
                                new Patch(m_opts, m_vars, m_uMin, m_uMax, vMid, m_vMax)));
                }
            }
        }

        virtual void transform(const Mat4& trans)
        {
            m_vars->transform(trans);
        }

        virtual Box bound() const
        {
            Vec3 a,b,c,d;
            getCorners(a,b,c,d);
            Box bnd(a);
            bnd.extendBy(b);
            bnd.extendBy(c);
            bnd.extendBy(d);
            return bnd;
        }
};

#endif // SURFACES_H_INCLUDED
