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

inline int bspTreeNodeIndex(int x, int y, int depth)
{
    assert(depth >= 0);
    int index = 0;
    if(depth % 2 == 0)
        std::swap(x,y);
    // interlace bits of i and j
    for(int i = 0; i < depth; i+=2)
    {
        index |= ((x & 1) << i) | ((y & 1) << (i+1));
        x >>= 1;
        y >>= 1;
    }
    return index + ((1 << depth) - 1);
}


/// Data structure for geometry storage during splitting
///
/// The main design aim here is to allow the top piece of geometry to be
/// retrieved for any given bucket, independent of the order in which the
/// buckets are processed.
class SplitStore
{
    private:
        // Example: BSP tree of depth 2.
        //
        // Node numbering
        //
        //          0       level 0
        //         / \            .
        //        /   \           .
        //       1     2          1
        //      / \   / \         .
        //     3   4 5   6        2
        //
        //
        // spatial layout
        //
        //   |         x
        //  -+------------>
        //   |
        //   |   +-------+   +-------+   +-------+
        //   |   |       |   |   |   |   | 3 | 5 |
        // y |   |   0   |   | 1 | 2 |   |---|---|
        //   v   |       |   |   |   |   | 4 | 6 |
        //       +-------+   +-------+   +-------+
        //
        struct Node
        {
            std::vector<GeomHolderPtr> queue;
            //boost::mutex mutex;
        };
        std::vector<Node> m_nodes;

        int m_nleafx;    ///< number of tree leaves in x-direction
        int m_nleafy;    ///< number of tree leaves in y-direction
        int m_treeDepth; ///< depth of the tree
        Imath::Box2f m_bound; ///< Bounding box of the root node

        static bool geomHeapOrder(const GeomHolderPtr& a,
                                  const GeomHolderPtr& b)
        {
            return a->bound().min.z > b->bound().min.z;
        }

    public:
        typedef int NodeId;

        SplitStore(int nleafx, int nleafy, const Imath::Box2f& bound)
            : m_nodes(),
            m_nleafx(nleafx),
            m_nleafy(nleafy),
            m_treeDepth(0),
            m_bound(bound)
        {
            m_treeDepth = std::max(2*iceil(log2(nleafx)) - 1,
                                   2*iceil(log2(nleafy)));
            m_nodes.resize((1 << (m_treeDepth+1)) - 1);
        }

        NodeId getNodeId(int i, int j) const
        {
            assert(i >= 0 && i < m_nleafx);
            assert(j >= 0 && j < m_nleafy);
            return bspTreeNodeIndex(i, j, m_treeDepth);
        }

        int nleafx() const { return m_nleafx; }
        int nleafy() const { return m_nleafy; }

        GeomHolderPtr pop(NodeId leaf)
        {
            // Search from leaf node down to the root, finding the geometry
            // closest to the camera.
            assert(leaf >= (1 << m_treeDepth) - 1);
            assert(leaf < (1 << (m_treeDepth+1)) - 1);
            float zmin = FLT_MAX;
            NodeId nodeIdToPop = -1;
            NodeId id = leaf;
            while(true)
            {
                std::vector<GeomHolderPtr>& queue = m_nodes[id].queue;
                // Discard any invalid geometry refs
                while(!queue.empty() && queue[0]->expired())
                {
                    std::pop_heap(queue.begin(), queue.end(), geomHeapOrder);
                    queue.pop_back();
                }
                if(!queue.empty() && queue[0]->bound().min.z < zmin)
                {
                    // record node where the closest geometry is
                    zmin = queue[0]->bound().min.z;
                    nodeIdToPop = id;
                }
                if(!id)
                    break;
                id = (id - 1) >> 1;
            }
            if(nodeIdToPop < 0)
                return GeomHolderPtr();
            std::vector<GeomHolderPtr>& queue = m_nodes[nodeIdToPop].queue;
            std::pop_heap(queue.begin(), queue.end(), geomHeapOrder);
            GeomHolderPtr result = queue.back();
            queue.pop_back();
            return result;
        }

        void insert(const GeomHolderPtr& geom)
        {
            Box& b = geom->bound();
            if(!m_bound.intersects(Imath::Box2f(vec2_cast(b.min),
                                                vec2_cast(b.max))))
                return;
            // Compute tree level at which the geometry will be inserted.  We
            // choose a tree level such that the geometry is smaller than the
            // tree nodes at that level.  This ensures that the geometry is
            // inserted into at most four nodes.
            //
            // Since we split in the x-direction first, the xdepth is one less
            // than the ydepth even when bound size is equal in the two
            // directions.
            float width = m_bound.max.x - m_bound.min.x;
            float height = m_bound.max.y - m_bound.min.y;
            int xdepth = -2*int(log2((b.max.x - b.min.x)/width));
            int ydepth = -2*int(log2((b.max.y - b.min.y)/height)) + 1;
            int depth = clamp(std::min(xdepth, ydepth), 0, m_treeDepth);
            // Compute integer (x,y) coordinates of the nodes which the bound
            // crosses at the insertion depth
            //
            // TODO: Is there a bug here with the (m_treeDepth - depth)/2 stuff?
            int x0 = (clamp(int(m_nleafx*(b.min.x - m_bound.min.x)/width),
                            0, m_nleafx-1)) >> (m_treeDepth - depth)/2;
            int x1 = (clamp(int(m_nleafx*(b.max.x - m_bound.min.x)/width),
                            0, m_nleafx-1)) >> (m_treeDepth - depth)/2;
            int y0 = (clamp(int(m_nleafy*(b.min.y - m_bound.min.y)/height),
                            0, m_nleafy-1)) >> (m_treeDepth - depth)/2;
            int y1 = (clamp(int(m_nleafy*(b.max.y - m_bound.min.y)/height),
                            0, m_nleafy-1)) >> (m_treeDepth - depth)/2;
            assert(x1-x0 <= 1);
            assert(y1-y0 <= 1);
            // Place geometry into nodes which it touches.
            for(int j = y0; j <= y1; ++j)
            {
                for(int i = x0; i <= x1; ++i)
                {
                    int index = bspTreeNodeIndex(i, j, depth);
                    std::vector<GeomHolderPtr>& queue = m_nodes[index].queue;
                    queue.push_back(geom);
                    std::push_heap(queue.begin(), queue.end(), geomHeapOrder);
                }
            }
        }
};


/*
void processBuckets()
{
    for(i in (left,right))
    {
        for(j in (top, bottom))
        {
            GeomStore::NodeId id = geomStore.getId(i,j);
            GeomStore::NodeId holderLoc;
            GeomHolderPtr g;
            while(g = geomStore.pop(id, &holderLoc))
            {
                geomStore.insert(g.split());
            }
        }
    }
}
*/

#endif // AQSIS_SPLITSTORE_H_INCLUDED

// vi:set et:
