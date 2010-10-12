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
#include "thread.h"
#include "util.h"
#include "varspec.h"

//------------------------------------------------------------------------------
/// Container for single geometry piece and associated metadata
///
/// This is the place to put everything about a piece of geometry which is
/// independent from the geometry type and needs to be cached for rendering
/// efficiency.
class GeomHolder : public RefCounted
{
    private:
        GeometryPtr m_geom;  ///< Main geometry (first key for deformation)
        GeometryKeys m_geomKeys; ///< Extra geometry keys
        int m_splitCount;    ///< Number of times the geometry has been split
        Box m_bound;         ///< Bound in camera coordinates
        ConstAttributesPtr m_attrs; ///< Surface attribute state
        bool m_hasChildren;  ///< True if child geometry/grid exists
        std::vector<GeomHolderPtr> m_childGeoms; ///< Child geometry
        GridHolderPtr m_childGrid;  ///< Child grid
        Mutex m_mutex;       ///< Mutex used when splitting

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
        GeomHolder(const GeometryPtr& geom, const ConstAttributesPtr& attrs)
            : m_geom(geom),
            m_geomKeys(),
            m_splitCount(0),
            m_bound(geom->bound()),
            m_attrs(attrs),
            m_hasChildren(false)
        { }

        /// Create geometry resulting from splitting (has a parent surface)
        GeomHolder(const GeometryPtr& geom, const GeomHolder& parent)
            : m_geom(geom),
            m_geomKeys(),
            m_splitCount(parent.m_splitCount+1),
            m_bound(geom->bound()),
            m_attrs(parent.m_attrs),
            m_hasChildren(false)
        { }

        /// Create initial deforming geometry (no parent surface)
        GeomHolder(GeometryKeys& keys, const ConstAttributesPtr& attrs)
            : m_geom(),
            m_geomKeys(keys),
            m_splitCount(0),
            m_bound(boundFromKeys(m_geomKeys)),
            m_attrs(attrs),
            m_hasChildren(false)
        { }

        /// Create deforming geometry resulting from splitting
        GeomHolder(GeometryPtr* keysBegin, GeometryPtr* keysEnd,
                   int keysStride, const GeomHolder& parent)
            : m_geom(),
            m_geomKeys(),
            m_splitCount(parent.m_splitCount+1),
            m_bound(),
            m_attrs(parent.m_attrs),
            m_hasChildren(false)
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
        bool isDeforming() const { return !m_geom; }
        int splitCount() const    { return m_splitCount; }
        Box& bound() { return m_bound; }
        const Attributes& attrs() const { return *m_attrs; }

        /// Has the geometry already been split/diced?
        bool hasChildren() const { return m_hasChildren; }
        /// Add some child geometry to the set of splitting results.
        void addChild(const GeomHolderPtr& childGeom)
        {
            m_childGeoms.push_back(childGeom);
        }
        /// Set the child grid as the result of dicing.
        void addChild(GridHolderPtr childGrid)
        {
            assert(!m_childGrid);
            m_childGrid = childGrid;
        }

        /// Clean up after tessellation is done.
        ///
        /// This should disassociate any held geometry, and set the flag
        /// indicating that child geometry is present.
        void tessellateFinished()
        {
            m_geom.reset();
            m_geomKeys.clear();
            m_hasChildren = true;
        }

        /// Get child grid if tessellation resulted in dicing.
        const GridHolderPtr& childGrid()        { return m_childGrid; }
        /// Get child geometry if tessellation resulted in splitting.
        std::vector<GeomHolderPtr>& childGeoms() { return m_childGeoms; }

        /// Get mutex used to protect setting of tessellated children.
        Mutex& mutex() { return m_mutex; }
};



//------------------------------------------------------------------------------
typedef MotionKey<GridPtr> GridKey;
typedef std::vector<GridKey> GridKeys;


/// Class to hold a grid of microgeometry, caching various properties.
///
/// This is the place to put everything about a grid which is independent from
/// the grid type and needs to be cached for rendering efficiency.  For
/// example, the raster bound is cached here rather than being recomputed by
/// the grid each time it's needed.
class GridHolder : public RefCounted
{
    private:
        GridPtr m_grid;             ///< Non-deforming grid
        GridKeys m_gridKeys;        ///< Grid keys for motion blur
        ConstAttributesPtr m_attrs; ///< Attribute state
        Box m_bound;                ///< Grid bounding box in raster coords.
        bool m_rasterized;          ///< True if the grid was rasterized

        void shade(Grid& grid) const
        {
            if(m_attrs->surfaceShader)
                m_attrs->surfaceShader->shade(grid);
        }

        void cacheBound()
        {
            if(isDeforming())
            {
                for(int i = 0, iend = m_gridKeys.size(); i < iend; ++i)
                    m_bound.extendBy(m_gridKeys[i].value->bound());
            }
            else
                m_bound.extendBy(m_grid->bound());
        }

    public:
        GridHolder(const GridPtr& grid, const GeomHolder& parentGeom)
            : m_grid(grid),
            m_gridKeys(),
            m_attrs(&parentGeom.attrs()),
            m_rasterized(false)
        {
            cacheBound();
        }

        template<typename GridPtrIterT>
        GridHolder(GridPtrIterT begin, GridPtrIterT end,
                   const GeomHolder& parentGeom)
            : m_grid(),
            m_gridKeys(),
            m_attrs(&parentGeom.attrs()),
            m_rasterized(false)
        {
            GeometryKeys::const_iterator oldKey = parentGeom.geomKeys().begin();
            m_gridKeys.reserve(end - begin);
            for(;begin != end; ++begin, ++oldKey)
                m_gridKeys.push_back(GridKey(oldKey->time, *begin));
            cacheBound();
        }

        bool isDeforming() const { return !m_grid; }
        Grid& grid() { return m_grid ? *m_grid : *m_gridKeys[0].value; }
        const Grid& grid() const { return m_grid ? *m_grid : *m_gridKeys[0].value; }
        GridKeys& gridKeys() { return m_gridKeys; }
        const GridKeys& gridKeys() const { return m_gridKeys; }
        const Box& bound() const { return m_bound; }

        const Attributes& attrs() const { return *m_attrs; }

        void setRasterized() { m_rasterized = true; }
        bool rasterized() const { return m_rasterized; }
};


//------------------------------------------------------------------------------
// Tessellation driver for rasterization
class TessellationContextImpl : public TessellationContext
{
    private:
        Renderer& m_renderer;         ///< Renderer instance
        GridStorageBuilder m_builder; ///< Grid allocator
        GeomHolder* m_currGeom;       ///< Geometry currently being split

        // Storage for partly tessellated
        std::vector<GeometryPtr> m_splits;
        std::vector<GridPtr> m_grids;

    public:
        TessellationContextImpl(Renderer& renderer);

        void tessellate(const Mat4& splitTrans, const GeomHolderPtr& holder);


        // From TessellationContext:
        virtual void invokeTessellator(TessControl& tessControl);

        virtual void push(const GeometryPtr& geom);
        virtual void push(const GridPtr& grid);

        virtual const Options& options();
        virtual const Attributes& attributes();

        virtual GridStorageBuilder& gridStorageBuilder();
};


#endif // AQSIS_TESSELLATION_H_INCLUDED
