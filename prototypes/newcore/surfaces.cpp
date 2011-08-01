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

/// \file
/// \author Chris Foster [chris42f (at) gmail (dot) com]

#include "surfaces.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// BilinearPatch implementation
BilinearPatch::BilinearPatch(const PrimvarStoragePtr& vars)
    : m_vars(vars),
    m_uMin(0), m_uMax(1),
    m_vMin(0), m_vMax(1)
{ }

bool BilinearPatch::motionCompatible(Geometry& geom)
{
    BilinearPatch* patch = dynamic_cast<BilinearPatch*>(&geom);
    if(!patch)
        return false;
    // Any two bilinear patches are automatically compatible, so
    // if the cast succeeds there's nothing else to check.
    return true;
}

void BilinearPatch::tessellate(const M44f& splitTrans, int forceSplit,
                               TessellationContext& tessCtx) const
{
    if(forceSplit)
    {
        // Forced split.  if this is an eye split.  We alternate split
        // direction to try to resolve eye splits.
        SurfaceSplitter<BilinearPatch> splitter(forceSplit % 2 == 0);
        tessCtx.invokeTessellator(splitter);
        return;
    }
    V3f a,b,c,d;
    getCorners(a,b,c,d);

    // Transform points into "splitting coordinates"
    V3f aSpl = a * splitTrans;
    V3f bSpl = b * splitTrans;
    V3f cSpl = c * splitTrans;
    V3f dSpl = d * splitTrans;

    const Options& opts = tessCtx.options();

    // estimate length in a-b, c-d direction
    float lu = 0.5*((bSpl-aSpl).length() + (dSpl-cSpl).length());
    // estimate length in a-c, b-d direction
    float lv = 0.5*((cSpl-aSpl).length() + (dSpl-bSpl).length());

    // Diceable test: Compare the number of vertices in the resulting
    // grid to the desired maximum grid size
    if(lu*lv <= opts.gridSize*opts.gridSize)
    {
        // Dice the surface when number of verts is small enough.
        int nu = 2 + ifloor(lu);
        int nv = 2 + ifloor(lv);
        SurfaceDicer<BilinearPatch> dicer(nu, nv);
        tessCtx.invokeTessellator(dicer);
    }
    else
    {
        // Otherwise, split the surface.  The splitting direction is
        // the shortest edge.
        bool splitDirectionU = lu > lv;
        SurfaceSplitter<BilinearPatch> splitter(splitDirectionU);
        tessCtx.invokeTessellator(splitter);
    }
}

Box3f BilinearPatch::bound() const
{
    V3f a,b,c,d;
    getCorners(a,b,c,d);
    Box3f bnd(a);
    bnd.extendBy(b);
    bnd.extendBy(c);
    bnd.extendBy(d);
    return bnd;
}


inline BilinearPatch::BilinearPatch(const PrimvarStoragePtr& vars,
                        float uMin, float uMax, float vMin, float vMax)
    : m_vars(vars),
    m_uMin(uMin), m_uMax(uMax),
    m_vMin(vMin), m_vMax(vMax)
{ }


/// Turn the patch into a grid of micropolygons
void BilinearPatch::dice(int nu, int nv, TessellationContext& tessCtx) const
{
    GridStorageBuilder& builder = tessCtx.gridStorageBuilder();
    // Add all the primvars to the grid
    builder.add(m_vars->varSet());
    GridStoragePtr storage = builder.build(nu*nv);
    boost::intrusive_ptr<QuadGrid> grid(new QuadGrid(nu, nv, storage));

    // Create some space to store the variable temporaries.
    int maxAgg = storage->maxAggregateSize();
    float* aMin = FALLOCA(maxAgg);
    float* aMax = FALLOCA(maxAgg);

    float du = (m_uMax-m_uMin)/(nu-1);
    float dv = (m_vMax-m_vMin)/(nv-1);

    for(int ivar = 0, nvars = m_vars->varSet().size();
        ivar < nvars; ++ivar)
    {
        ConstFvecView pvar = m_vars->get(ivar);
        FvecView gvar = storage->get(m_vars->varSet()[ivar]);
        int size = gvar.elSize();

        if(gvar.uniform())
        {
            // Uniform - no interpolation, just copy.
            std::memcpy(gvar[0], pvar[0], size*sizeof(float));
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
    tessCtx.push(grid);
}

/// Split the patch into two sub-patches
void BilinearPatch::split(bool splitInU, TessellationContext& tessCtx) const
{
    // Split
    if(splitInU)
    {
        // split in the middle of the a-b and c-d sides.
        // a---b
        // | | |
        // c---d
        float uMid = 0.5*(m_uMin + m_uMax);
        tessCtx.push(GeometryPtr(new BilinearPatch(m_vars, m_uMin,
                                                   uMid, m_vMin, m_vMax)));
        tessCtx.push(GeometryPtr(new BilinearPatch(m_vars, uMid,
                                                   m_uMax, m_vMin, m_vMax)));
    }
    else
    {
        // split in the middle of the a-c and b-d sides.
        // a---b
        // |---|
        // c---d
        float vMid = 0.5*(m_vMin + m_vMax);
        tessCtx.push(GeometryPtr(new BilinearPatch(m_vars, m_uMin,
                                                   m_uMax, m_vMin, vMid)));
        tessCtx.push(GeometryPtr(new BilinearPatch(m_vars, m_uMin,
                                                   m_uMax, vMid, m_vMax)));
    }
}

/// Get the corner vertices of the patch
inline void BilinearPatch::getCorners(V3f& a, V3f& b, V3f& c, V3f& d) const
{
    ConstDataView<V3f> P = m_vars->P();
    a = bilerp(P[0], P[1], P[2], P[3], m_uMin, m_vMin);
    b = bilerp(P[0], P[1], P[2], P[3], m_uMax, m_vMin);
    c = bilerp(P[0], P[1], P[2], P[3], m_uMin, m_vMax);
    d = bilerp(P[0], P[1], P[2], P[3], m_uMax, m_vMax);
}


//------------------------------------------------------------------------------
// ConvexPolyMesh implementation

ConvexPolyMesh::ConvexPolyMesh(int nFaces, const int* vertsPerFace,
                               int nVertexIndices, const int* vertexIndices,
                               const PrimvarStoragePtr& vars)
    : m_vars(vars),
    m_nfaces(nFaces),
    m_vertsPerFace(new int[m_nfaces]),
    m_nverts(nVertexIndices),
    m_vertIndices(new int[m_nverts])
{
    std::memcpy(m_vertsPerFace.get(), vertsPerFace, sizeof(int)*m_nfaces);
    std::memcpy(m_vertIndices.get(), vertexIndices, sizeof(int)*m_nverts);
}

bool ConvexPolyMesh::motionCompatible(Geometry& geom)
{
    ConvexPolyMesh* mesh = dynamic_cast<ConvexPolyMesh*>(&geom);
    if(!mesh)
        return false;
    // Ok, the other geometry is a mesh.  Also need to check that the
    // number of verts per face are the same for both meshes.
    if(m_nfaces != mesh->m_nfaces)
        return false;
    for(int i = 0; i < m_nfaces; ++i)
        if(m_vertsPerFace[i] != mesh->m_vertsPerFace[i])
            return false;
    return true;
}

struct ConvexPolyMesh::Splitter : public TessControl
{
    virtual void tessellate(const Geometry& geom,
                            TessellationContext& tessContext) const
    {
        static_cast<const ConvexPolyMesh&>(geom).split(tessContext);
    }
};

void ConvexPolyMesh::tessellate(const M44f& splitTrans, int forceSplit,
                        TessellationContext& tessCtx) const
{
    Splitter splitter;
    tessCtx.invokeTessellator(splitter);
}

Box3f ConvexPolyMesh::bound() const
{
    Box3f bnd;
    ConstDataView<V3f> P = m_vars->P();
    int numVerts = m_vars->iclassStorage().vertex;
    for(int i = 0; i < numVerts; ++i)
        bnd.extendBy(P[i]);
    return bnd;
}

/// Split the mesh into a piece of geometry for each face
///
/// TODO: For good performance this should be much more sophisticated!
void ConvexPolyMesh::split(TessellationContext& tessCtx) const
{
    const PrimvarSet& varSpecs = m_vars->varSet();
    // Copy parameters from the mesh to each patch.
    PrimvarStorageBuilder builder;
    for(int face = 0, vertsIdx = 0; face < m_nfaces;
        vertsIdx += m_vertsPerFace[face], ++face)
    {
        if(m_vertsPerFace[face] != 4)
        {
            // TODO: Non-quad faces need to be supported!
            continue;
        }
        // Construct arrays holding the indices for the current patch in the
        // mesh privar arrays.
        const int constantIndices[] = {0};
        const int uniformIndices[] = {face};
        const int fvertexIndices[] = {vertsIdx, vertsIdx + 1,
                                      vertsIdx + 3, vertsIdx + 2};
        const int vertexIndices[] = {
            m_vertIndices[fvertexIndices[0]], m_vertIndices[fvertexIndices[1]],
            m_vertIndices[fvertexIndices[2]], m_vertIndices[fvertexIndices[3]]
        };
        const int* allIndices[] = {constantIndices, uniformIndices,
                                   vertexIndices, vertexIndices,
                                   fvertexIndices, fvertexIndices};
        for(int i = 0; i < varSpecs.size(); ++i)
        {
            builder.add(varSpecs[i], m_vars->get(i),
                        allIndices[varSpecs[i].iclass]);
        }
        PrimvarStoragePtr primVars = builder.build(IclassStorage(1,4,4,4,4));
        tessCtx.push(new BilinearPatch(primVars));
    }
}

}
