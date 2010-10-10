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
                getBucket(V2i(i,j)).bound() =
                    Imath::Box2f(bucketMin, bucketMax);
            }
        }

        /// Get number of buckets in the x-direction
        int nxBuckets() const { return m_nxBuckets; }
        /// Get number of buckets in the y-direction
        int nyBuckets() const { return m_nyBuckets; }

        /// Insert geometry into the data structure
        ///
        /// The geometric bound is used to determine which buckets to insert
        /// into.
        void insert(const GeomHolderPtr& geom)
        {
            int x0 = 0, x1 = 0, y0 = 0, y1 = 0;
            if(!bucketRangeForBound(geom->bound(), x0, x1, y0, y1))
                return;
            // Place geometry into nodes which it touches.
            for(int j = y0; j <= y1; ++j)
                for(int i = x0; i <= x1; ++i)
                    getBucket(V2i(i,j)).insert(geom);
        }

        /// Geometry storage for a bucket.
        class Bucket
        {
            public:
                Bucket() : m_queue(), m_queueInit(false), m_bound() { }

                /// Grab the top surface for the given bucket
                ///
                /// If there are no surfaces present in the bucket, return
                /// null.
                GeomHolder* popSurface()
                {
                    if(!m_queueInit)
                    {
                        // Initialize priority queue with _raw_ pointers
                        // extracted from the geometry list.  Using raw
                        // pointers rather than reference counted ones here is
                        // very important for performance of the queue
                        // operations!
                        m_queue.reserve(m_geoms.size());
                        for(int i = 0, iend = m_geoms.size(); i < iend; ++i)
                            m_queue.push_back(m_geoms[i].get());
                        std::make_heap(m_queue.begin(), m_queue.end(),
                                       geomHeapOrder);
                        m_queueInit = true;
                    }
                    if(m_queue.empty())
                        return 0;
                    std::pop_heap(m_queue.begin(), m_queue.end(),
                                  geomHeapOrder);
                    GeomHolder* result = m_queue.back();
                    m_queue.pop_back();
                    return result;
                }

                /// Push surface back onto bucket, after checking the bound.
                void pushSurface(GeomHolder* geom)
                {
                    Box& gbnd = geom->bound();
                    if(gbnd.min.x < m_bound.max.x  &&
                       gbnd.min.y < m_bound.max.y  &&
                       gbnd.max.x >= m_bound.min.x &&
                       gbnd.max.y >= m_bound.min.y)
                    {
                        m_queue.push_back(geom);
                        std::push_heap(m_queue.begin(), m_queue.end(),
                                        geomHeapOrder);
                    }
                }

                /// Indicate that the bucket is rendered.
                ///
                /// This deallocates bucket resources.
                void setFinished()
                {
                    assert(m_queue.empty());
                    // Force vector to clear memory.  Note that this is
                    // important - failing to do so can result in memory
                    // fragmentation due to the small but long-lived chunk of
                    // memory wasted here.
                    vectorFree(m_queue);
                    vectorFree(m_geoms);
                }

            private:
                friend class SplitStore;

                /// Insert geometry unconditionally into the bucket.
                void insert(const GeomHolderPtr& geom)
                {
                    m_geoms.push_back(geom);
                }

                /// Get bucket bound
                Imath::Box2f& bound() { return m_bound; }

                static bool geomHeapOrder(GeomHolder* a, GeomHolder* b)
                {
                    return a->bound().min.z > b->bound().min.z;
                }

                /// Storage for initial geometry provided through the API
                std::vector<GeomHolderPtr> m_geoms;
                /// geometry queue.  We avoid priority_queue here in favour of
                /// std::vector so that we can force the held memory to be
                /// freed.
                std::vector<GeomHolder*> m_queue;
                bool m_queueInit;     ///< true if m_queue is initialized
                Imath::Box2f m_bound; ///< Raster bound for the bucket
        };

        /// Get identifier for leaf bucket at position pos.
        Bucket& getBucket(V2i pos)
        {
            return m_buckets[pos.y*m_nxBuckets + pos.x];
        }

    private:
        /// Get range of buckets for the given bound.
        bool bucketRangeForBound(const Box& bnd, int& x0, int& x1,
                                 int& y0, int& y1) const
        {
            if(!m_bound.intersects(Imath::Box2f(vec2_cast(bnd.min),
                                                vec2_cast(bnd.max))))
                return false;
            float width = m_bound.max.x - m_bound.min.x;
            float height = m_bound.max.y - m_bound.min.y;
            // Compute integer (x,y) coordinates of the nodes which the
            // geometry bound touches at the computed insertion depth
            x0 = clamp(int(m_nxBuckets*(bnd.min.x - m_bound.min.x)/width),
                       0, m_nxBuckets-1);
            x1 = clamp(int(m_nxBuckets*(bnd.max.x - m_bound.min.x)/width),
                       0, m_nxBuckets-1);
            y0 = clamp(int(m_nyBuckets*(bnd.min.y - m_bound.min.y)/height),
                       0, m_nyBuckets-1);
            y1 = clamp(int(m_nyBuckets*(bnd.max.y - m_bound.min.y)/height),
                       0, m_nyBuckets-1);
            return true;
        }

        boost::scoped_array<Bucket> m_buckets; ///< geometry storage
        int m_nxBuckets;      ///< number of buckets in x-direction
        int m_nyBuckets;      ///< number of buckets in y-direction
        Imath::Box2f m_bound; ///< Bounding box
};


#endif // AQSIS_SPLITSTORE_H_INCLUDED

// vi:set et:
