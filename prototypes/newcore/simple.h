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

#ifndef SIMPLE_H_INCLUDED
#define SIMPLE_H_INCLUDED

#include "attributes.h"
#include "geometry.h"
#include "invbilin.h"
#include "options.h"
#include "pointinquad.h"
#include "util.h"

//------------------------------------------------------------------------------
/// A 2D grid of quadrilateral micro polygons
class QuadGridSimple : public Grid
{
    public:
        QuadGridSimple(int nu, int nv)
            : Grid(GridType_QuadSimple),
            m_nu(nu),
            m_nv(nv),
            m_P(nu*nv)
        { }

        virtual GridStorage& storage()
        {
            return *m_storage;
        }

        virtual void calculateNormals(DataView<Vec3> N,
                                      ConstDataView<Vec3> P) const
        { }

        Vec3& vertex(int u, int v) { return m_P[m_nu*v + u]; }
        Vec3 vertex(int u, int v) const { return m_P[m_nu*v + u]; }

        int nu() const { return m_nu; }
        int nv() const { return m_nv; }

        Vec3* P(int v) { assert(v >= 0 && v < m_nv); return &m_P[m_nu*v]; }

        virtual void project(const Mat4& m)
        {
            for(int i = 0, iend = m_P.size(); i < iend; ++i)
            {
                float z = m_P[i].z;
                m_P[i] = m_P[i]*m;
                m_P[i].z = z;
            }
        }

    private:
        int m_nu;
        int m_nv;
        std::vector<Vec3> m_P;
        static GridStorage* m_storage; ///< Incredibly bogus, but necessary to
                                       ///  fulfil full grid interface!
};



//------------------------------------------------------------------------------
class PatchSimple : public Geometry
{
    private:
        Vec3 m_P[4];

        friend class SurfaceSplitter<PatchSimple>;
        friend class SurfaceDicer<PatchSimple>;

        void dice(int uRes, int vRes, TessellationContext& tessCtx) const
        {
            boost::intrusive_ptr<QuadGridSimple>
                grid(new QuadGridSimple(uRes, vRes));
            float dv = 1.0f/(vRes-1);
            float du = 1.0f/(uRes-1);
            for(int v = 0; v < vRes; ++v)
            {
                Vec3 Pmin = Imath::lerp(m_P[0], m_P[2], v*dv);
                Vec3 Pmax = Imath::lerp(m_P[1], m_P[3], v*dv);
                Vec3* row = grid->P(v);
                for(int u = 0; u < uRes; ++u)
                    row[u] = Imath::lerp(Pmin, Pmax, u*du);
            }
            tessCtx.push(grid);
        }

        void split(bool splitInU, TessellationContext& tessCtx) const
        {
            if(splitInU)
            {
                // split in the middle of the a-b and c-d sides.
                // a---b
                // | | |
                // c---d
                Vec3 ab = 0.5f*(m_P[0] + m_P[1]);
                Vec3 cd = 0.5f*(m_P[2] + m_P[3]);
                tessCtx.push(GeometryPtr(
                            new PatchSimple(m_P[0], ab, m_P[2], cd)));
                tessCtx.push(GeometryPtr(
                            new PatchSimple(ab, m_P[1], cd, m_P[3])));
            }
            else
            {
                // split in the middle of the a-c and b-d sides.
                // a---b
                // |---|
                // c---d
                Vec3 ac = 0.5f*(m_P[0] + m_P[2]);
                Vec3 bd = 0.5f*(m_P[1] + m_P[3]);
                tessCtx.push(GeometryPtr(
                            new PatchSimple(m_P[0], m_P[1], ac, bd)));
                tessCtx.push(GeometryPtr(
                            new PatchSimple(ac, bd, m_P[2], m_P[3])));
            }
        }

    public:
        PatchSimple(Vec3 a, Vec3 b, Vec3 c, Vec3 d)
        {
            m_P[0] = a; m_P[1] = b;
            m_P[2] = c; m_P[3] = d;
        }

        virtual void tessellate(const Mat4& splitTrans,
                                TessellationContext& tessCtx) const
        {
            // Project points into "splitting coordinates"
            Vec3 a = m_P[0] * splitTrans;
            Vec3 b = m_P[1] * splitTrans;
            Vec3 c = m_P[2] * splitTrans;
            Vec3 d = m_P[3] * splitTrans;

            // Diceable test: Estimate area as the sum of the areas of two
            // triangles which make up the patch.
            float area = 0.5 * (  ((b-a)%(c-a)).length()
                                + ((b-d)%(c-d)).length() );

            const Options& opts = tessCtx.options();
            const float maxArea = opts.gridSize*opts.gridSize;

            // estimate length in a-b, c-d direction
            float lu = 0.5*((b-a).length() + (d-c).length());
            // estimate length in a-c, b-d direction
            float lv = 0.5*((c-a).length() + (d-b).length());

            if(area <= maxArea)
            {
                // When the area (in number of micropolys) is small enough,
                // dice the surface.
                int uRes = 1 + ifloor(lu);
                int vRes = 1 + ifloor(lv);
                SurfaceDicer<PatchSimple> dicer(uRes, vRes);
                tessCtx.invokeTessellator(dicer);
            }
            else
            {
                // Otherwise, split the surface.  The splitting direction is
                // the shortest edge.
                SurfaceSplitter<PatchSimple> splitter(lu > lv);
                tessCtx.invokeTessellator(splitter);
            }
        }

        virtual void transform(const Mat4& trans)
        {
            m_P[0] *= trans;
            m_P[1] *= trans;
            m_P[2] *= trans;
            m_P[3] *= trans;
        }

        virtual Box bound() const
        {
            Box b(m_P[0]);
            b.extendBy(m_P[1]);
            b.extendBy(m_P[2]);
            b.extendBy(m_P[3]);
            return b;
        }
};


#endif // SIMPLE_H_INCLUDED
