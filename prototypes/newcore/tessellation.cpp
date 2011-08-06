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

/// \file Tessellation context implementation and grid/geom holders
/// \author Chris Foster [chris42f (at) gmail (d0t) com]

#include "tessellation.h"

namespace Aqsis {

TessellationContextImpl::TessellationContextImpl(Renderer& renderer,
                                ResourceCounterStat<>& geomsInFlight,
                                ResourceCounterStat<>& gridsInFlight)
    : m_shadingContext(renderer.m_camToWorld),
    m_renderer(renderer),
    m_geomsInFlight(geomsInFlight),
    m_gridsInFlight(gridsInFlight),
    m_builder(),
    m_currGeom(0)
{ }

void TessellationContextImpl::tessellate(const M44f& splitTrans,
                                         const GeomHolderPtr& holder)
{
    m_currGeom = holder.get();
    // Grab an exclusive lock for the current geometry so that no other thread
    // tries to tessellate it concurrently
    LockGuard lk(m_currGeom->mutex());
    if(m_currGeom->hasChildren())
    {
        // Another thread has already split/diced the geometry so there's
        // nothing to do here.
        return;
    }
    int forceSplit = 0;
    // Force split rather than dice if the bound spans the epsilon plane.
    if(holder->rasterBound().min.z < FLT_EPSILON)
        forceSplit = holder->splitCount();
    holder->geom().tessellate(splitTrans, forceSplit, *this);
    m_currGeom->tessellateFinished();
}


/// Add child geometry to a parent's split results list if it can't be culled.
void TessellationContextImpl::addChildGeometry(GeomHolder& parent,
                                               const GeomHolderPtr& child) const
{
    // First check to see whether the child can be culled completely
    const Box2i parentBound = parent.bucketBound();
    if(!m_renderer.rasterCull(*child, &parentBound))
    {
        // Copy over the occlusion record from the parent
        if(child->copyOcclusionRecord(parent))
        {
            ++m_geomsInFlight;
            // Only add if child bound was partially unoccluded according to
            // parent geometry occlusion record.
            parent.addChild(child);
        }
    }
}

void TessellationContextImpl::invokeTessellator(TessControl& tessControl)
{
    // Collect the split/dice results in m_splits/m_grids.
    m_splits.clear();
    m_grids.clear();
    int splitsPerKey = 0;
    // First, do the actual tessellation by invoking the tessellator
    // control object on the geometry.
    if(m_currGeom->isDeforming())
    {
        const GeometryKeys& keys = m_currGeom->geomKeys();
        tessControl.tessellate(*keys[0].value, *this);
        splitsPerKey = m_splits.size();
        for(int i = 1, nkeys = keys.size(); i < nkeys; ++i)
            tessControl.tessellate(*keys[i].value, *this);
    }
    else
    {
        // Non-deforming case.
        tessControl.tessellate(m_currGeom->geom(), *this);
    }
    if(!m_grids.empty())
    {
        if(Shader* dispShader = m_currGeom->attrs().displacementShader.get())
        {
            // Shade all grids with the displacement shader
            for(int i = 0; i < (int)m_grids.size(); ++i)
                dispShader->shade(m_shadingContext, *m_grids[i]);
        }
        // Shade only the primary motion grid using the surface shader.
        if(Shader* surfShader = m_currGeom->attrs().surfaceShader.get())
            surfShader->shade(m_shadingContext, *m_grids[0]);
        // Project grids
        for(int i = 0; i < (int)m_grids.size(); ++i)
            m_grids[i]->project(m_renderer.m_camToSRaster);
    }
    if(m_currGeom->isDeforming())
    {
        // Deforming case - gather together the split/dice results
        // produced by the current deforming surface set
        if(!m_splits.empty())
        {
            assert(m_splits.size() % splitsPerKey == 0);
            for(int i = 0; i < splitsPerKey; ++i)
            {
                GeomHolderPtr holder(new GeomHolder(
                                &*m_splits.begin() + i,
                                &*m_splits.end() + i,
                                splitsPerKey, *m_currGeom));
                addChildGeometry(*m_currGeom, holder);
            }
        }
        if(!m_grids.empty())
        {
            GridHolderPtr gridh(new GridHolder(m_grids.begin(), m_grids.end(),
                                               *m_currGeom));
            if(!m_renderer.rasterCull(*gridh, m_currGeom->bucketBound()))
            {
                ++m_gridsInFlight;
                m_currGeom->addChild(gridh);
            }
        }
    }
    else
    {
        // Static non-deforming case.
        if(!m_splits.empty())
        {
            // Push surfaces back to the renderer
            for(int i = 0, iend = m_splits.size(); i < iend; ++i)
            {
                GeomHolderPtr holder(new GeomHolder(m_splits[i], *m_currGeom));
                addChildGeometry(*m_currGeom, holder);
            }
        }
        if(!m_grids.empty())
        {
            assert(m_grids.size() == 1);
            GridHolderPtr gridh(new GridHolder(m_grids[0], *m_currGeom));
            if(!m_renderer.rasterCull(*gridh, m_currGeom->bucketBound()))
            {
                ++m_gridsInFlight;
                m_currGeom->addChild(gridh);
            }
        }
    }
}

void TessellationContextImpl::push(const GeometryPtr& geom)
{
    m_splits.push_back(geom);
}

void TessellationContextImpl::push(const GridPtr& grid)
{
    // Fill in any grid data which didn't get filled in by the surface
    // during the dicing stage.
    //
    // TODO: Alias optimization:
    //  - For perspective projections I may be aliased to P rather
    //    than copied
    //  - N may sometimes be aliased to Ng
    //
    GridStorage& stor = grid->storage();
    DataView<V3f> P = stor.get(StdIndices::P);
    // Deal with normals N & Ng
    DataView<V3f> Ng = stor.get(StdIndices::Ng);
    DataView<V3f> N = stor.get(StdIndices::N);
    if(Ng)
        grid->calculateNormals(Ng, P);
    if(N && !m_builder.dicedByGeom(stor, StdIndices::N))
    {
        if(Ng)
            copy(N, Ng, stor.nverts());
        else
            grid->calculateNormals(N, P);
    }
    // Deal with view direction.
    if(DataView<V3f> I = stor.get(StdIndices::I))
    {
        // In shading coordinates, I is just equal to P for
        // perspective projections.  (TODO: orthographic)
        copy(I, P, stor.nverts());
    }
    m_grids.push_back(grid);
}

const Options& TessellationContextImpl::options()
{
    return *m_renderer.m_opts;
}

const Attributes& TessellationContextImpl::attributes()
{
    return m_currGeom->attrs();
}

GridStorageBuilder& TessellationContextImpl::gridStorageBuilder()
{
    // Add required stdvars for sampling, shader input & output.
    //
    // TODO: Perhaps some of this messy logic can be done once & cached
    // in the surface holder?
    //
    m_builder.clear();
    // TODO: AOV stuff shouldn't be conditional on surfaceShader
    // existing.
    if(m_currGeom->attrs().surfaceShader)
    {
        // Renderer arbitrary output vars
        const OutvarSet& aoVars = m_renderer.m_outVars;
        const Shader& shader = *m_currGeom->attrs().surfaceShader;
        // TODO: Need to also consider displacement shader vars
        const VarSet& inVars = shader.inputVars();
        // P is guaranteed to be dice by the geometry.
        m_builder.add(Stdvar::P,  GridStorage::Varying);
        // Add stdvars computed by the renderer if they're needed in
        // the shader or AOVs.  These are:
        //
        //   I - computed from P
        //   du, dv - compute from u,v
        //   E - eye position is always 0
        //   ncomps - computed from options
        //   time - always 0 (?)
        if(inVars.contains(StdIndices::I) || aoVars.contains(Stdvar::I))
            m_builder.add(Stdvar::I,  GridStorage::Varying);
        // TODO: du, dv, E, ncomps, time

        // Some geometric stdvars - dPdu, dPdv, Ng - may in principle
        // be filled in by the geometry.  For now we just estimate
        // these in the renderer core using derivatives of P.
        //
        // TODO: dPdu, dPdv
        if(inVars.contains(Stdvar::Ng) || aoVars.contains(Stdvar::Ng))
            m_builder.add(Stdvar::Ng,  GridStorage::Varying);
        // N is a tricky case; it may inherit a value from Ng if it's
        // not set explicitly, but only if Ng is never assigned to by
        // the shaders (implicitly via P).
        //
        // Thoughts about variable flow for N:
        //
        // How can N differ from Ng??
        // - If it's a primvar
        // - If N is changed in the displacement shader
        // - If P is changed in the displacement shader (implies Ng is
        //   too)
        //
        // 1) Add N if contained in inVars or outVars or AOVs
        // 2) Add if it's in the primvar list
        // 3) Allocate if N can differ from Ng, *and* both N & Ng are
        //    in the list.
        // 4) Dice if it's a primvar
        // 5) Alias to Ng if N & Ng can't differ, else memcpy.
        //
        // Lesson: N and Ng should be the same, unless N is specified
        // by the user (ie, is a primvar) or N & Ng diverge during
        // displacement (ie, N is set or P is set)
        if(inVars.contains(Stdvar::N) || aoVars.contains(Stdvar::N))
            m_builder.add(Stdvar::N,  GridStorage::Varying);

        // Stdvars which should be attached at geometry creation:
        // Cs, Os - from attributes state
        // u, v
        // s, t - copy u,v ?
        //
        // Stdvars which must be filled in by the geometry:
        // P
        //
        // Stdvars which can be filled in by either geometry or
        // renderer.  Here's how you'd do them with the renderer:
        // N - computed from Ng
        // dPdu, dPdv - computed from P
        // Ng - computed from P

        // Add shader outputs
        const VarSet& outVars = shader.outputVars();
        if(outVars.contains(StdIndices::Ci) && aoVars.contains(Stdvar::Ci))
            m_builder.add(Stdvar::Ci, GridStorage::Varying);
        if(outVars.contains(StdIndices::Oi) && aoVars.contains(Stdvar::Oi))
            m_builder.add(Stdvar::Oi, GridStorage::Varying);
        // TODO: Replace the limited stuff above with the following:
        /*
        for(var in outVars)
        {
            // TODO: signal somehow that these vars are to be retained
            // after shading.
            if(aovs.contains(var))
                m_builder.add(var);
        }
        */
    }
    m_builder.setFromGeom();
    return m_builder;
}

} // namespace Aqsis
