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

#ifndef AQSIS_TESSELLATION_H_INCLUDED
#define AQSIS_TESSELLATION_H_INCLUDED

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "attributes.h"
#include "geometry.h"
#include "grid.h"
#include "gridstorage.h"
#include "options.h"
#include "refcount.h"
#include "renderer.h"
#include "stats.h"
#include "thread.h"
#include "util.h"
#include "varspec.h"

namespace Aqsis {

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
        Box3f m_rasterBound; ///< Bound in camera coordinates
        ConstAttributesPtr m_attrs; ///< Surface attribute state
        volatile bool m_hasChildren;  ///< True if child geometry/grid exists
        std::vector<GeomHolderPtr> m_childGeoms; ///< Child geometry
        GridHolderPtr m_childGrid;  ///< Child grid
        Mutex m_mutex;       ///< Mutex used when splitting
        volatile boost::uint32_t m_bucketRefs;  ///< Number of buckets referencing this geometry.  atomic.
        V2i m_bboundStart;   ///< First bucket which the bound touches
        V2i m_bboundSize;    ///< Number of buckets the bound touches
        boost::dynamic_bitset<> m_occludedBuckets; ///< Buckets in which the geometry was occluded.

        /// Get the bound from a set of geometry keys
        static Box3f boundFromKeys(const GeometryKeys& keys)
        {
            Box3f bound = keys[0].value->bound();
            for(int i = 1, iend = keys.size(); i < iend; ++i)
                bound.extendBy(keys[i].value->bound());
            return bound;
        }

        /// Return true if all children have been deleted.
        ///
        /// (This is a debug tool.)
        bool childrenDeleted() const
        {
            bool deleted = true;
            for(int i = 0, nchildren = m_childGeoms.size(); i < nchildren; ++i)
                deleted &= !m_childGeoms[i];
            // TODO: Also check grid.
            return deleted;
        }

    public:
        /// Create initial non-deforming geometry (no parent surface)
        GeomHolder(const GeometryPtr& geom, const ConstAttributesPtr& attrs)
            : m_geom(geom),
            m_geomKeys(),
            m_splitCount(0),
            m_rasterBound(geom->bound()),
            m_attrs(attrs),
            m_hasChildren(false),
            m_bucketRefs(0)
        { }

        /// Create geometry resulting from splitting (has a parent surface)
        GeomHolder(const GeometryPtr& geom, const GeomHolder& parent)
            : m_geom(geom),
            m_geomKeys(),
            m_splitCount(parent.m_splitCount+1),
            m_rasterBound(geom->bound()),
            m_attrs(parent.m_attrs),
            m_hasChildren(false),
            m_bucketRefs(0)
        { }

        /// Create initial deforming geometry (no parent surface)
        GeomHolder(GeometryKeys& keys, const ConstAttributesPtr& attrs)
            : m_geom(),
            m_geomKeys(keys),
            m_splitCount(0),
            m_rasterBound(boundFromKeys(m_geomKeys)),
            m_attrs(attrs),
            m_hasChildren(false),
            m_bucketRefs(0)
        { }

        /// Create deforming geometry resulting from splitting
        GeomHolder(GeometryPtr* keysBegin, GeometryPtr* keysEnd,
                   int keysStride, const GeomHolder& parent)
            : m_geom(),
            m_geomKeys(),
            m_splitCount(parent.m_splitCount+1),
            m_rasterBound(),
            m_attrs(parent.m_attrs),
            m_hasChildren(false),
            m_bucketRefs(0)
        {
            // Init geom keys, taking every keysStride'th geometry
            assert(static_cast<int>(parent.geomKeys().size())
                   == (keysEnd-keysBegin)/keysStride);
            m_geomKeys.reserve(parent.geomKeys().size());
            GeometryKeys::const_iterator oldKey = parent.geomKeys().begin();
            for(; keysBegin < keysEnd; keysBegin += keysStride, ++oldKey)
                m_geomKeys.push_back(GeometryKey(oldKey->time, *keysBegin));
            m_rasterBound = boundFromKeys(m_geomKeys);
        }

        ~GeomHolder()
        {
            // Check that all split children have already been deleted.  If
            // this check fails, it means that we are failing to delete the
            // leaves of the split tree as soon as possible.
            assert(!m_hasChildren || (m_hasChildren && childrenDeleted()));
        }

        //------------------------------------------------------------

        /// Get non-deforming geometry or first key frame
        const Geometry& geom() const
        {
            return m_geom ? *m_geom : *m_geomKeys[0].value;
        }

        /// Get deforming geometry key frames.  Empty if non-moving.
        const GeometryKeys& geomKeys() const { return m_geomKeys; }

        /// Return true if this holds deforming geometry
        bool isDeforming() const { return !m_geom; }

        /// Return the number of times the geometry has been split
        int splitCount() const   { return m_splitCount; }

        /// Return the geometry bound.
        ///
        /// This is initially in world space, but is transformed to combined
        /// sraster/camera z space immediately after creation.
        const Box3f& rasterBound() const { return m_rasterBound; }
        /// Cache the raster bound for later use.
        void setRasterBound(Box3f bnd) { m_rasterBound = bnd; }

        /// Get discrete bucket bound for the geometry (exclusive end!)
        ///
        /// The bucket bound is the bound of the geometry as transformed into
        /// discrete bucket coordinates.  This is only valid after
        /// initBucketRefs() has been called.
        Box2i bucketBound() const
        {
            return Box2i(m_bboundStart, m_bboundStart + m_bboundSize);
        }

        /// Return the attribute state associated with the geometry.  Threadsafe.
        const Attributes& attrs() const { return *m_attrs; }

        /// Has the geometry already been split/diced?  Threadsafe.
        bool hasChildren() const { return m_hasChildren; }

        /// Get child grid if tessellation resulted in dicing.  Threadsafe.
        const GridHolderPtr& childGrid() const { return m_childGrid; }
        GridHolderPtr& childGrid()             { return m_childGrid; }

        /// Get child geometry if tessellation resulted in splitting.
        ///
        /// Threadsafe, provided hasChildren() returns true.
        std::vector<GeomHolderPtr>& childGeoms() { return m_childGeoms; }

        /// Set flag indicating that the geometry was occluded in the given bucket.
        ///
        /// pos - position of bucket.
        ///
        /// Returns true if the flag was successfully set.  A return value of
        /// false indicates that the geometry was already split.
        ///
        /// Threadsafe.
        bool setOccludedInBucket(V2i pos)
        {
            LockGuard lk(m_mutex);
            if(m_hasChildren)
                return false;
            if(m_occludedBuckets.empty())
                m_occludedBuckets.resize(prod(m_bboundSize));
            m_occludedBuckets[m_bboundSize.x*(pos.y - m_bboundStart.y) +
                              pos.x - m_bboundStart.x] = true;
            return true;
        }

        /// Release a bucket reference, returning true if the count decreased
        /// to zero.
        ///
        /// This is intended to be a threadsafe atomic operation.
        bool releaseBucketRef()
        {
            return atomic_dec32(&m_bucketRefs) - 1 == 0;
        }

        //------------------------------------------------------------
        /// \group Tessellation utilities
        ///
        /// These functions should be called only by the tessellation context.
        //@{

        /// Get mutex used to protect setting of tessellated children.
        Mutex& mutex() { return m_mutex; }

        /// Add some child geometry to the set of splitting results.
        ///
        /// The geometry mutex must be locked before calling this function.
        void addChild(const GeomHolderPtr& childGeom)
        {
            m_childGeoms.push_back(childGeom);
        }

        /// Set the child grid as the result of dicing.
        ///
        /// The geometry mutex must be locked before calling this function.
        void addChild(GridHolderPtr childGrid)
        {
            assert(!m_childGrid);
            m_childGrid = childGrid;
        }

        /// Clean up after tessellation is done.
        ///
        /// This should disassociate any held geometry, and set the flag
        /// indicating that child geometry is present.
        ///
        /// The geometry mutex must be locked before calling this function.
        void tessellateFinished()
        {
            m_geom.reset();
            vectorFree(m_geomKeys);
            // Free the memory held by the occlusion record bitset
            boost::dynamic_bitset<>().swap(m_occludedBuckets);
            m_hasChildren = true;
        }

        /// Set the number of bucket references.
        ///
        /// Should happen directly after construction, before other threads
        /// have a chance to touch *this.
        void initBucketRefs(V2i bucketBegin, V2i bucketEnd)
        {
            m_bboundStart = bucketBegin;
            m_bboundSize = bucketEnd - bucketBegin;
            m_bucketRefs = prod(m_bboundSize);
        }

        /// Copy the occlusion record of the parent geometry.
        ///
        /// The occlusion record is an array of bits, one for each bucket
        /// which the geometry bound touches.  Each time a piece of geometry is
        /// occluded in a bucket, we set the appropriate bit.  This is
        /// necessary so that when splitting the child geometry can get the
        /// correct reference count
        ///
        /// parent - geometry from which to copy the occlusion record.  The
        ///          parent bound must be larger than (or equal to) the size of
        ///          the current bound.
        bool copyOcclusionRecord(GeomHolder& parent)
        {
            if(parent.m_occludedBuckets.empty())
                return true;
            m_occludedBuckets.resize(prod(m_bboundSize));
            V2i offset = m_bboundStart - parent.m_bboundStart;
            // Note that this loop could probably be a lot more efficient if we
            // copied blocks rather than individual bits.
            for(int j = 0; j < m_bboundSize.y; ++j)
            for(int i = 0; i < m_bboundSize.x; ++i)
            {
                m_occludedBuckets[j*m_bboundSize.x + i] = parent.m_occludedBuckets[
                        parent.m_bboundSize.x*(j + offset.y) + i + offset.x];
            }
            assert(m_bucketRefs >= m_occludedBuckets.count());
            m_bucketRefs -= m_occludedBuckets.count();
            return m_bucketRefs != 0;
        }
        //@}
};



//------------------------------------------------------------------------------
struct GridKey
{
    float time;
    Box3f bound;
    std::vector<Box3f> cachedBounds;
    GridPtr grid;
};
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
        Box3f m_rasterBound;        ///< Raster bounding box including CoC expansion
        Box3f m_tightBound;         ///< Geometric raster bounding box
        bool m_rasterized;          ///< True if the grid was rasterized
        volatile boost::uint32_t m_bucketRefs; ///< Number of buckets referencing this grid.  must be atomic.
        Box2i m_bucketBound;        ///< Bound in bucket coordinates.
        std::vector<Box3f> m_cachedBounds; ///< Cached micropolygon bounds

    public:
        GridHolder(const GridPtr& grid, const GeomHolder& parentGeom)
            : m_grid(grid),
            m_gridKeys(),
            m_attrs(&parentGeom.attrs()),
            m_rasterized(false),
            m_bucketRefs(0)
        {
            m_tightBound = m_grid->bound();
            m_grid->cacheBounds(m_cachedBounds);
        }

        template<typename GridPtrIterT>
        GridHolder(GridPtrIterT begin, GridPtrIterT end,
                   const GeomHolder& parentGeom)
            : m_grid(),
            m_gridKeys(),
            m_attrs(&parentGeom.attrs()),
            m_rasterized(false),
            m_bucketRefs(0)
        {
            GeometryKeys::const_iterator oldKey = parentGeom.geomKeys().begin();
            m_gridKeys.resize(end - begin);
            for(int i = 0; begin != end; ++begin, ++oldKey, ++i)
            {
                m_gridKeys[i].time = oldKey->time;
                m_gridKeys[i].grid = *begin;
                m_gridKeys[i].bound = (*begin)->bound();
                m_tightBound.extendBy(m_gridKeys[i].bound);
                (*begin)->cacheBounds(m_gridKeys[i].cachedBounds);
            }
        }

        bool isDeforming() const { return !m_grid; }
        Grid& grid() { return m_grid ? *m_grid : *m_gridKeys[0].grid; }
        const Grid& grid() const { return m_grid ? *m_grid : *m_gridKeys[0].grid; }
        GridKeys& gridKeys() { return m_gridKeys; }
        const GridKeys& gridKeys() const { return m_gridKeys; }
        /// Get the bound in sraster space.
        const Box3f& rasterBound() const { return m_rasterBound; }
        /// Get the tight bound in sraster space (no expansion for depth of field).
        const Box3f& tightRasterBound() const { return m_tightBound; }
        const std::vector<Box3f>& cachedBounds() const { return m_cachedBounds; }

        const Attributes& attrs() const { return *m_attrs; }

        void setRasterized() { m_rasterized = true; }
        bool rasterized() const { return m_rasterized; }

        /// Initialize the number of bucket references.
        void initBucketRefs(V2i bucketBegin, V2i bucketEnd)
        {
            m_bucketBound = Box2i(bucketBegin, bucketEnd);
            m_bucketRefs = prod(bucketEnd - bucketBegin);
        }

        /// Cache the raster bound for later use.
        void setRasterBound(Box3f bnd) { m_rasterBound = bnd; }

        /// Get discrete bucket bound for the geometry (exclusive end!)
        ///
        /// The bucket bound indicates which buckets the bound of the grid
        /// touches.  This is only valid after initBucketRefs() has been
        /// called.
        Box2i bucketBound() const
        {
            return m_bucketBound;
        }

        /// Release a bucket reference, returning true if the count decreased
        /// to zero.  Threadsafe.
        bool releaseBucketRef()
        {
            return atomic_dec32(&m_bucketRefs) - 1 == 0;
        }
};


//------------------------------------------------------------------------------
// Tessellation driver for rasterization
class TessellationContextImpl : public TessellationContext
{
    public:
        TessellationContextImpl(Renderer& renderer,
                                ResourceCounterStat<>& geomsInFlight,
                                ResourceCounterStat<>& gridsInFlight);

        void tessellate(const M44f& splitTrans, const GeomHolderPtr& holder);


        // From TessellationContext:
        virtual void invokeTessellator(TessControl& tessControl);

        virtual void push(const GeometryPtr& geom);
        virtual void push(const GridPtr& grid);

        virtual const Options& options();
        virtual const Attributes& attributes();

        virtual GridStorageBuilder& gridStorageBuilder();

    private:
        void addChildGeometry(GeomHolder& parent,
                              const GeomHolderPtr& child) const;

        ShadingContext m_shadingContext;
        Renderer& m_renderer;         ///< Renderer instance
        mutable ResourceCounterStat<>& m_geomsInFlight;
        mutable ResourceCounterStat<>& m_gridsInFlight;
        GridStorageBuilder m_builder; ///< Grid allocator
        GeomHolder* m_currGeom;       ///< Geometry currently being split

        // Storage for partly tessellated
        std::vector<GeometryPtr> m_splits;
        std::vector<GridPtr> m_grids;
};


} // namespace Aqsis
#endif // AQSIS_TESSELLATION_H_INCLUDED
