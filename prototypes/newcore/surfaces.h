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

#ifndef AQSIS_SURFACES_H_INCLUDED
#define AQSIS_SURFACES_H_INCLUDED

#include "attributes.h"
#include "geometry.h"
#include "options.h"
#include "util.h"

#include "primvar.h"
#include "gridstorage.h"
#include "grid.h"

namespace Aqsis {

/// Bilinear patch surface type.
///
/// The bilinear patch is defined by bilinear interpolation between four corner
/// vertices:
///
///   P(u,v) = lerp(lerp(P1, P2, u), lerp(P3, P4, u), v)
///
/// where the vertices P1 to P4 are laid out in the standard "patch order",
///
/// P1 -- P2
/// |     |
/// |     |
/// P3 -- P4
///
/// (note that this is different from the usual cyclic ordering of vertices
/// used for polygons)
///
/// For the bilinear patch, all the variable interpolation classes vertex,
/// varying, facevertex and facevarying are the same, and use the bilinear
/// vertex interpolation rule discussed above.  The uniform and constant
/// classes are constant across the patch.
///
class BilinearPatch : public Geometry
{
    public:
        BilinearPatch(const PrimvarStoragePtr& vars);

        virtual bool motionCompatible(Geometry& geom);

        virtual void tessellate(const M44f& splitTrans, int forceSplit,
                                TessellationContext& tessCtx) const;

        virtual Box3f bound() const;

    private:
        friend class SurfaceSplitter<BilinearPatch>;
        friend class SurfaceDicer<BilinearPatch>;

        BilinearPatch(const PrimvarStoragePtr& vars,
                      float uMin, float uMax, float vMin, float vMax);

        void dice(int nu, int nv, TessellationContext& tessCtx) const;

        void split(bool splitInU, TessellationContext& tessCtx) const;

        void getCorners(V3f& a, V3f& b, V3f& c, V3f& d) const;

        PrimvarStoragePtr m_vars;
        // uv coordinates for corners of the base patch.
        const float m_uMin, m_uMax;
        const float m_vMin, m_vMax;
};


//------------------------------------------------------------------------------
/// A mesh of convex polygons.
///
/// This class corresponds to the PointsPolygons API call.
///
/// TODO: This is a work in progress, there are several major caveats:
///   - Only quad faces are supported
///   - Small polygons will be very inefficiently handled because a separate
///     piece of geometry is made for each face.
///   - The split tree is very broad, since splitting ConvexPolyMesh
///     immediately results in a piece of geometry for each face.  This will
///     bog down the geometry handling part of the pipeline.
class ConvexPolyMesh : public Geometry
{
    public:
        ConvexPolyMesh(int nFaces, const int* vertsPerFace,
                       int nVertexIndices, const int* vertexIndices,
                       const PrimvarStoragePtr& vars);

        virtual bool motionCompatible(Geometry& geom);

        virtual void tessellate(const M44f& splitTrans, int forceSplit,
                                TessellationContext& tessCtx) const;

        virtual Box3f bound() const;

    private:
        struct Splitter;

        void split(TessellationContext& tessCtx) const;

        PrimvarStoragePtr m_vars;
        int m_nfaces;
        boost::scoped_array<int> m_vertsPerFace;
        int m_nverts;
        boost::scoped_array<int> m_vertIndices;
};


} // namespace Aqsis
#endif // AQSIS_SURFACES_H_INCLUDED
