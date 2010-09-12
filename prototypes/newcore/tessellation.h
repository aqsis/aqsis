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

#ifndef AQSIS_TESSELLATION_H_INCLUDED
#define AQSIS_TESSELLATION_H_INCLUDED

#include <vector>

#include "attributes.h"
#include "geometry.h"
#include "grid.h"
#include "gridstorage.h"
#include "options.h"
#include "refcount.h"
#include "renderer.h"
#include "util.h"
#include "varspec.h"

//------------------------------------------------------------------------------
/// Container for single geometry piece and associated metadata
class GeomHolder : public RefCounted
{
    private:
        GeometryPtr m_geom;  ///< Main geometry (first key for deformation)
        GeometryKeys m_geomKeys; ///< Extra geometry keys
        int m_splitCount;    ///< Number of times the geometry has been split
        Box m_bound;         ///< Bound in camera coordinates
        Attributes* m_attrs; ///< Surface attribute state

        /// Get the bound from a set of geometry keys
        static Box boundFromKeys(const GeometryKeys& keys)
        {
            Box bound = keys[0].value->bound();
            for(int i = 1, iend = keys.size(); i < iend; ++i)
                bound.extendBy(keys[i].value->bound());
            return bound;
        }

    public:
        /// Create initial non-deforming geometry (no parent surface)
        GeomHolder(const GeometryPtr& geom, Attributes* attrs)
            : m_geom(geom),
            m_geomKeys(),
            m_splitCount(0),
            m_bound(geom->bound()),
            m_attrs(attrs)
        { }
        /// Create geometry resulting from splitting (has a parent surface)
        GeomHolder(const GeometryPtr& geom, const GeomHolder& parent)
            : m_geom(geom),
            m_geomKeys(),
            m_splitCount(parent.m_splitCount+1),
            m_bound(geom->bound()),
            m_attrs(parent.m_attrs)
        { }

        /// Create initial deforming geometry (no parent surface)
        GeomHolder(GeometryKeys& keys, Attributes* attrs)
            : m_geom(),
            m_geomKeys(keys),
            m_splitCount(0),
            m_bound(boundFromKeys(m_geomKeys)),
            m_attrs(attrs)
        { }
        /// Create deforming geometry resulting from splitting
        GeomHolder(GeometryPtr* keysBegin, GeometryPtr* keysEnd,
                   int keysStride, const GeomHolder& parent)
            : m_geom(),
            m_geomKeys(),
            m_splitCount(parent.m_splitCount+1),
            m_bound(),
            m_attrs(parent.m_attrs)
        {
            // Init geom keys, taking every keysStride'th geometry
            assert(static_cast<int>(parent.geomKeys().size())
                   == (keysEnd-keysBegin)/keysStride);
            m_geomKeys.reserve(parent.geomKeys().size());
            GeometryKeys::const_iterator oldKey = parent.geomKeys().begin();
            for(; keysBegin < keysEnd; keysBegin += keysStride, ++oldKey)
                m_geomKeys.push_back(GeometryKey(oldKey->time, *keysBegin));
            m_bound = boundFromKeys(m_geomKeys);
        }

        /// Get non-deforming geometry or first key frame
        const Geometry& geom() const
        {
            return m_geom ? *m_geom : *m_geomKeys[0].value;
        }

        /// Get deforming geometry key frames.  Empty if non-moving.
        const GeometryKeys& geomKeys() const { return m_geomKeys; }

        /// True if the holder no longer holds valid geometry
        bool expired() const { return !m_geom && m_geomKeys.empty(); }
        bool isDeforming() const { return !m_geom; }
        int splitCount() const    { return m_splitCount; }
        Box& bound() { return m_bound; }
        Attributes& attrs()       { return *m_attrs; }
        const Attributes& attrs() const { return *m_attrs; }
};



//------------------------------------------------------------------------------
typedef MotionKey<GridPtr> GridKey;
typedef std::vector<GridKey> GridKeys;

class GridHolder : public RefCounted
{
    private:
        GridPtr m_grid;            ///< Non-deforming grid
        GridKeys m_gridKeys;       ///< Grid keys for motion blur
        const Attributes* m_attrs; ///< Attribute state

        void shade(Grid& grid) const
        {
            // Special case: can't shade simple grids.
            if(grid.type() == GridType_QuadSimple)
                return;
            if(m_attrs->surfaceShader)
                m_attrs->surfaceShader->shade(grid);
        }

    public:
        GridHolder(const GridPtr& grid, const Attributes* attrs)
            : m_grid(grid),
            m_gridKeys(),
            m_attrs(attrs)
        { }

        template<typename GridPtrIterT>
        GridHolder(GridPtrIterT begin, GridPtrIterT end,
                   const GeomHolder& parentGeom)
            : m_grid(),
            m_gridKeys(),
            m_attrs(&parentGeom.attrs())
        {
            GeometryKeys::const_iterator oldKey = parentGeom.geomKeys().begin();
            m_gridKeys.reserve(end - begin);
            for(;begin != end; ++begin, ++oldKey)
                m_gridKeys.push_back(GridKey(oldKey->time, *begin));
        }

        bool isDeforming() const { return !m_grid; }
        Grid& grid() { return m_grid ? *m_grid : *m_gridKeys[0].value; }
        GridKeys& gridKeys() { return m_gridKeys; }

        const Attributes& attrs() const { return *m_attrs; }

        /// Shade all grids
        void shade()
        {
            if(isDeforming())
            {
                // TODO: Run displacement shaders only on extra grids
                // TODO: Fill in time variable on the grid before shading
                for(int i = 0, iend = m_gridKeys.size(); i < iend; ++i)
                    shade(*m_gridKeys[i].value);
            }
            else
                shade(*m_grid);
        }

        /// Project all grids held by the holder
        void project(const Mat4& camToRas)
        {
            if(isDeforming())
            {
                for(int i = 0, iend = m_gridKeys.size(); i < iend; ++i)
                    m_gridKeys[i].value->project(camToRas);
            }
            else
                m_grid->project(camToRas);
        }
};


//------------------------------------------------------------------------------
// Minimal wrapper around a renderer instance to provide control context for
// when surfaces push split/diced objects back into the render's queue.
class TessellationContextImpl : public TessellationContext
{
    private:
        Renderer& m_renderer;
        GridStorageBuilder m_builder;
        const GeomHolder* m_parentGeom;

        /// Collecter for splits of deforming surfaces
        class DeformCollector
        {
            private:
                std::vector<GeometryPtr> m_splits;
                int m_splitsPerKey;
                std::vector<GridPtr> m_grids;

            public:
                void reset()
                {
                    m_splits.clear();
                    m_splitsPerKey = 0;
                    m_grids.clear();
                }

                void firstKeyDone()
                {
                    m_splitsPerKey = m_splits.size();
                }

                void push(const GeometryPtr& geom)
                {
                    m_splits.push_back(geom);
                }

                void push(const GridPtr& grid)
                {
                    m_grids.push_back(grid);
                }

                void pushResults(const GeomHolder& parentGeom,
                                 Renderer& renderer)
                {
                    if(!m_splits.empty())
                    {
                        // Construct holder for each deforming child surface &
                        // push it back to the renderer.
                        assert((m_splits.size()/m_splitsPerKey)*m_splitsPerKey
                                == m_splits.size());
                        for(int i = 0; i < m_splitsPerKey; ++i)
                        {
                            GeomHolderPtr holder(
                                new GeomHolder(
                                    &*m_splits.begin() + i,
                                    &*m_splits.end() + i,
                                    m_splitsPerKey, parentGeom
                                )
                            );
                            renderer.push(holder);
                        }
                    }
                    if(!m_grids.empty())
                    {
                        // Construct deforming child grids
                        renderer.push( GridHolderPtr(
                                new GridHolder(
                                    m_grids.begin(), m_grids.end(),
                                    parentGeom
                                )
                        ));
                    }
                }
        };
        DeformCollector m_deformCollector;

    public:
        TessellationContextImpl(Renderer& renderer)
            : m_renderer(renderer),
            m_builder(),
            m_parentGeom(0)
        { }

        void tessellate(const Mat4& splitTrans, float polyLength,
                        const GeomHolderPtr& holder)
        {
            m_parentGeom = holder.get();
            holder->geom().tessellate(splitTrans, polyLength, *this);
        }

        virtual void invokeTessellator(TessControl& tessControl)
        {
            if(m_parentGeom->isDeforming())
            {
                // Collect the split/dice results in m_deformCollector if the
                // surface is involved in deformation motion blur.
                m_deformCollector.reset();
                const GeometryKeys& keys = m_parentGeom->geomKeys();
                tessControl.tessellate(*keys[0].value, *this);
                m_deformCollector.firstKeyDone();
                for(int i = 1, nkeys = keys.size(); i < nkeys; ++i)
                    tessControl.tessellate(*keys[i].value, *this);
                m_deformCollector.pushResults(*m_parentGeom, m_renderer);
            }
            else
                tessControl.tessellate(m_parentGeom->geom(), *this);
        }

        virtual void push(const GeometryPtr& geom)
        {
            if(m_parentGeom->isDeforming())
                m_deformCollector.push(geom);
            else
            {
                GeomHolderPtr holder(new GeomHolder(geom, *m_parentGeom));
                m_renderer.push(holder);
            }
        }

        virtual void push(const GridPtr& grid)
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
            DataView<Vec3> P = stor.get(StdIndices::P);
            // Deal with normals N & Ng
            DataView<Vec3> Ng = stor.get(StdIndices::Ng);
            DataView<Vec3> N = stor.get(StdIndices::N);
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
            if(DataView<Vec3> I = stor.get(StdIndices::I))
            {
                // In shading coordinates, I is just equal to P for
                // perspective projections.  (TODO: orthographic)
                copy(I, P, stor.nverts());
            }
            if(m_parentGeom->isDeforming())
                m_deformCollector.push(grid);
            else
            {
                m_renderer.push(GridHolderPtr(
                    new GridHolder(grid, &m_parentGeom->attrs()) ));
            }
        }

        virtual const Options& options()
        {
            return m_renderer.m_opts;
        }
        virtual const Attributes& attributes()
        {
            return m_parentGeom->attrs();
        }

        virtual GridStorageBuilder& gridStorageBuilder()
        {
            // Add required stdvars for sampling, shader input & output.
            //
            // TODO: Perhaps some of this messy logic can be done once & cached
            // in the surface holder?
            //
            m_builder.clear();
            // TODO: AOV stuff shouldn't be conditional on surfaceShader
            // existing.
            if(m_parentGeom->attrs().surfaceShader)
            {
                // Renderer arbitrary output vars
                const OutvarSet& aoVars = m_renderer.m_outVars;
                const Shader& shader = *m_parentGeom->attrs().surfaceShader;
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
};


#endif // AQSIS_TESSELLATION_H_INCLUDED
