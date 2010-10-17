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

/// \file Data structure for geometry storage
/// \author Chris Foster [chris42f (at) gmail (d0t) com]

#ifndef AQSIS_SPLITSTORE_H_INCLUDED
#define AQSIS_SPLITSTORE_H_INCLUDED

#include <algorithm>
#include <cassert>

#include <boost/scoped_array.hpp>

#include "tessellation.h"
#include "util.h"

namespace Aqsis {

class GeometryQueue;

//------------------------------------------------------------------------------
/// Data structure for geometry storage during splitting
///
/// The main design aim here is to allow the top piece of geometry to be
/// retrieved for any given bucket, independent of the order in which the
/// buckets are processed.
class SplitStore
{
    public:
        /// Create a storage structure with nxBuckets x nyBuckets buckets, and
        /// given spatial bound
        SplitStore(int nxBuckets, int nyBuckets, const Imath::Box2f& bound)
            : m_buckets(new Bucket[nxBuckets*nyBuckets]),
            m_nxBuckets(nxBuckets),
            m_nyBuckets(nyBuckets),
            m_bound(bound)
        {
            Vec2 bucketSize = (m_bound.max - m_bound.min) /
                              Vec2(nxBuckets, nyBuckets);
            for(int j = 0; j < nyBuckets; ++j)
            for(int i = 0; i < nxBuckets; ++i)
            {
                Vec2 bucketMin = m_bound.min + bucketSize*Vec2(i,j);
                Vec2 bucketMax = m_bound.min + bucketSize*Vec2(i+1,j+1);
                getBucket(V2i(i,j)).bound =
                    Imath::Box2f(bucketMin, bucketMax);
            }
        }

        /// Get number of buckets in the x-direction
        int nxBuckets() const { return m_nxBuckets; }
        /// Get number of buckets in the y-direction
        int nyBuckets() const { return m_nyBuckets; }

        /// Insert initial geometry into buckets
        ///
        /// The geometric bound is used to determine which buckets to insert
        /// into.  This function is not threadsafe!  It should be used only
        /// to associate root nodes in the splitting tree with buckets.
        void insert(const GeomHolderPtr& geom)
        {
            const Box& bnd = geom->bound();
            if(!m_bound.intersects(Imath::Box2f(vec2_cast(bnd.min),
                                                vec2_cast(bnd.max))))
                return;
            int x0 = 0, x1 = 0, y0 = 0, y1 = 0;
            bucketRangeForBound(bnd, x0, x1, y0, y1);
            // Place geometry into nodes which it touches.
            for(int j = y0; j < y1; ++j)
                for(int i = x0; i < x1; ++i)
                    getBucket(V2i(i,j)).geoms.push_back(geom);
        }

        /// Get range of buckets for the given bound.
        void bucketRangeForBound(const Box& bnd, int& x0, int& x1,
                                 int& y0, int& y1) const
        {
            float width = m_bound.max.x - m_bound.min.x;
            float height = m_bound.max.y - m_bound.min.y;
            // Compute integer (x,y) coordinates of the nodes which the
            // geometry bound touches at the computed insertion depth
            x0 = clamp(ifloor(m_nxBuckets*(bnd.min.x - m_bound.min.x)/width),
                       0, m_nxBuckets-1);
            x1 = clamp(ifloor(m_nxBuckets*(bnd.max.x - m_bound.min.x)/width) + 1,
                       0, m_nxBuckets);
            y0 = clamp(ifloor(m_nyBuckets*(bnd.min.y - m_bound.min.y)/height),
                       0, m_nyBuckets-1);
            y1 = clamp(ifloor(m_nyBuckets*(bnd.max.y - m_bound.min.y)/height) + 1,
                       0, m_nyBuckets);
        }

        /// Grab geometry from the bucket at the given position and put into
        /// the provided queue object.
        void enqueueGeometry(GeometryQueue& queue, const V2i& bucketPos);

    private:
        friend class GeometryQueue;

        /// Geometry storage for a bucket.
        struct Bucket
        {
            /// Indicate that the bucket is rendered.
            ///
            /// This deallocates bucket resources.
            void setFinished()
            {
                // Force vector to clear memory.  Note that this is
                // important - failing to do so can result in memory
                // fragmentation due to the small but long-lived chunk of
                // memory wasted here.
                vectorFree(geoms);
            }

            /// Storage for initial geometry provided through the API
            std::vector<GeomHolderPtr> geoms;
            Imath::Box2f bound; ///< Raster bound for the bucket
        };

        /// Get identifier for leaf bucket at position pos.
        Bucket& getBucket(V2i pos)
        {
            return m_buckets[pos.y*m_nxBuckets + pos.x];
        }

        boost::scoped_array<Bucket> m_buckets; ///< geometry storage
        int m_nxBuckets;      ///< number of buckets in x-direction
        int m_nyBuckets;      ///< number of buckets in y-direction
        Imath::Box2f m_bound; ///< Bounding box
};


//------------------------------------------------------------------------------
/// Priority queue of geometry on a bucket.
///
/// The queue grabs all geometry in a bucket and arranges it so that surfaces
/// close to the camera can be rendered first.  It also keeps a list of
/// each piece of geometry in the splitting tree which has been touched during
/// rendering so that the refcount can be decremented at the end of the
/// bucket.
class GeometryQueue
{
    public:
        /// Initialize the queue with surfaces from the given bucket.
        void enqueueBucket(SplitStore::Bucket& bucket)
        {
            m_toRelease.clear();
            m_bucket = &bucket;
            std::vector<GeomHolderPtr>& geoms = bucket.geoms;
            // Initialize priority queue with _raw_ pointers extracted from
            // the bucket's geometry list.  Using raw pointers rather than
            // reference counted ones here is very important for performance
            // of the queue operations!
            assert(m_queue.empty());
            m_queue.reserve(geoms.size());
            for(int i = 0, iend = geoms.size(); i < iend; ++i)
                m_queue.push_back(geoms[i].get());
            std::make_heap(m_queue.begin(), m_queue.end(), geomHeapOrder);
        }

        /// Grab the top piece of geometry (ie, that with minimum depth)
        ///
        /// If there is no geometry left, return null.
        GeomHolder* pop()
        {
            if(m_queue.empty())
                return 0;
            std::pop_heap(m_queue.begin(), m_queue.end(), geomHeapOrder);
            GeomHolder* result = m_queue.back();
            m_queue.pop_back();
            return result;
        }

        /// Push geometry back onto the queue, after checking the bound.
        ///
        /// If the bound touches the current bucket, the geometry is added to
        /// the priority queue and also added to a list of surfaces to release
        /// in releaseBucket().
        void push(GeomHolderPtr& geom)
        {
            if(!geom)
                return;
            const Imath::Box2f& bbnd = m_bucket->bound;
            Box& gbnd = geom->bound();
            if(gbnd.min.x < bbnd.max.x  && gbnd.min.y < bbnd.max.y  &&
               gbnd.max.x >= bbnd.min.x && gbnd.max.y >= bbnd.min.y)
            {
                m_queue.push_back(geom.get());
                std::push_heap(m_queue.begin(), m_queue.end(), geomHeapOrder);
                m_toRelease.push_back(&geom);
            }
        }

        /// Release bucket resources.
        ///
        /// This function releases references the surfaces store in the most
        /// recently used bucket.  It also decrements the reference counts of
        /// any internal nodes in the splitting tree and deletes them if
        /// they fall to zero.
        void releaseBucket()
        {
            // Release references to all surfaces which touched this bucket.
            // Note that the order is important here!  The split tree is
            // traversed from root to leaves during rendering, so leaves
            // appear last in the vector m_toRelease.
            for(int i = (int)m_toRelease.size() - 1; i >= 0; --i)
            {
                if((*m_toRelease[i])->releaseBucketRef())
                    m_toRelease[i]->reset();
            }
            m_bucket->setFinished();
        }

    private:
        /// Ordering functor for surface rendering priority
        static bool geomHeapOrder(GeomHolder* a, GeomHolder* b)
        {
            return a->bound().min.z > b->bound().min.z;
        }

        SplitStore::Bucket* m_bucket;
        std::vector<GeomHolder*> m_queue;  ///< geometry queue.
        std::vector<GeomHolderPtr*> m_toRelease;  ///< geometry queue.
};


void SplitStore::enqueueGeometry(GeometryQueue& queue, const V2i& bucketPos)
{
    queue.enqueueBucket(getBucket(bucketPos));
}

} // namespace Aqsis
#endif // AQSIS_SPLITSTORE_H_INCLUDED

// vi:set et:
