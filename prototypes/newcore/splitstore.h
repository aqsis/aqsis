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

#ifndef AQSIS_SPLITSTORE_H_INCLUDED
#define AQSIS_SPLITSTORE_H_INCLUDED

#include <algorithm>
#include <cassert>

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
        /// Create a storage structure with nxBuckets x nyBuckets buckets, and given
        /// spatial bound
        SplitStore(int nxBuckets, int nyBuckets, const Imath::Box2f& bound)
            : m_buckets(nxBuckets*nyBuckets),
            m_nxBuckets(nxBuckets),
            m_nyBuckets(nyBuckets),
            m_bound(bound)
        { }

        /// Get number of buckets in the x-direction
        int nxBuckets() const { return m_nxBuckets; }
        /// Get number of buckets in the y-direction
        int nyBuckets() const { return m_nyBuckets; }

        /// Grab the top surface for the given bucket
        ///
        /// If there are no surfaces present in the bucket, return null.
        GeomHolderPtr pop(int x, int y)
        {
            return getBucket(x,y).pop();
        }

        /// Insert geometry into the data structure
        ///
        /// The geometric bound is used to determine which buckets to insert
        /// into.
        void insert(const GeomHolderPtr& geom)
        {
            Box& b = geom->bound();
            if(!m_bound.intersects(Imath::Box2f(vec2_cast(b.min),
                                                vec2_cast(b.max))))
                return;
            float width = m_bound.max.x - m_bound.min.x;
            float height = m_bound.max.y - m_bound.min.y;
            // Compute integer (x,y) coordinates of the nodes which the
            // geometry bound touches at the computed insertion depth
            int x0 = (clamp(int(m_nxBuckets*(b.min.x - m_bound.min.x)/width),
                            0, m_nxBuckets-1));
            int x1 = (clamp(int(m_nxBuckets*(b.max.x - m_bound.min.x)/width),
                            0, m_nxBuckets-1));
            int y0 = (clamp(int(m_nyBuckets*(b.min.y - m_bound.min.y)/height),
                            0, m_nyBuckets-1));
            int y1 = (clamp(int(m_nyBuckets*(b.max.y - m_bound.min.y)/height),
                            0, m_nyBuckets-1));
            // Place geometry into nodes which it touches.
            for(int j = y0; j <= y1; ++j)
                for(int i = x0; i <= x1; ++i)
                    getBucket(i,j).insert(geom);
        }

    private:
        /// Queued geometry storage
        class Bucket
        {
            private:
                static bool isExpired(const GeomHolderPtr& g)
                {
                    return g->expired();
                }
                static bool geomHeapOrder(const GeomHolderPtr& a,
                                        const GeomHolderPtr& b)
                {
                    return a->bound().min.z > b->bound().min.z;
                }

                std::vector<GeomHolderPtr> m_queue; ///< geometry queue
                bool m_isHeap;     ///< true if m_queue is a heap
                int m_expireCheck; ///< position in m_queue to check for expired geoms

            public:
                Bucket()
                    : m_queue(),
                    m_isHeap(false),
                    m_expireCheck(0)
                {}

                /// Grab top piece of geometry from bucket.
                GeomHolderPtr pop()
                {
                    if(!m_isHeap)
                    {
                        // Discard all expired refs in one sweep of array, and
                        // subsequently make into a heap
                        m_queue.erase(std::remove_if(m_queue.begin(), m_queue.end(),
                                                     &isExpired), m_queue.end());
                        std::make_heap(m_queue.begin(), m_queue.end(), geomHeapOrder);
                        m_isHeap = true;
                    }
                    else
                    {
                        // Discard any invalid geometry refs
                        while(!m_queue.empty() && m_queue[0]->expired())
                        {
                            std::pop_heap(m_queue.begin(), m_queue.end(), geomHeapOrder);
                            m_queue.pop_back();
                        }
                    }
                    if(m_queue.empty())
                        return GeomHolderPtr();
                    std::pop_heap(m_queue.begin(), m_queue.end(), geomHeapOrder);
                    GeomHolderPtr result = m_queue.back();
                    m_queue.pop_back();
                    assert(!result->expired());
                    return result;
                }

                /// Insert geometry into bucket
                void insert(const GeomHolderPtr& geom)
                {
                    if(m_isHeap)
                    {
                        // If currently a heap (that is, we want to be able top
                        // pop() at any time), insert and maintain heap
                        // structure.
                        m_queue.push_back(geom);
                        std::push_heap(m_queue.begin(), m_queue.end(), geomHeapOrder);
                    }
                    else
                    {
                        // If m_queue has no special structure, do a quick
                        // check to find a storage location which has already
                        // expired, and use that if possible.
                        if(!m_queue.empty())
                        {
                            ++m_expireCheck;
                            if(m_expireCheck >= (int)m_queue.size())
                                m_expireCheck = 0;
                            if(m_queue[m_expireCheck]->expired())
                            {
                                m_queue[m_expireCheck] = geom;
                                return;
                            }
                        }
                        m_queue.push_back(geom);
                    }
                }
        };

        /// Get identifier for leaf bucket at position x,y
        Bucket& getBucket(int x, int y)
        {
            return m_buckets[y*m_nxBuckets + x];
        }

        std::vector<Bucket> m_buckets; ///< geometry storage
        int m_nxBuckets;      ///< number of buckets in x-direction
        int m_nyBuckets;      ///< number of buckets in y-direction
        Imath::Box2f m_bound; ///< Bounding box
};


#endif // AQSIS_SPLITSTORE_H_INCLUDED

// vi:set et:
