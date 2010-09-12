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

//------------------------------------------------------------------------------
// Quad tree utilities for indexing array-based node storage.
//
// We can store a balanced quad tree in an array if we put the root node at 0
// and the children of a node at index n at the indices 2*n+1, 2*n+2, 2*n+3,
// 2*n+4.  There's multiple ways to label the children spatially - here's the
// ordering we use:
//
// Numbers indicate node indices.
//
//             level 0            level 1             level 2
//
//        +---------------+   +-------+-------+   +-------+-------+
//        |               |   |       |       |   | 15| 16| 19| 20|
//   ^    |               |   |   3   |   4   |   |-------|-------|
//   |    |               |   |       |       |   | 13| 14| 17| 18|
// y |    |       0       |   +-------+-------+   +-------+-------+
//   |    |               |   |       |       |   | 7 | 8 | 11| 12|
//   |    |               |   |   1   |   2   |   |-------|-------|
//   |    |               |   |       |       |   | 5 | 6 | 9 | 10|
//   |    +---------------+   +-------+-------+   +-------+-------+
//   |
//   |           x
//  -+------------>
//   |
//
//

/// Get total number of nodes in a balanced quadtree
inline int quadTreeNumNodes(int depth)
{
    // This is just the sum of the number of nodes on each level, which
    // happens to be a geometric series.
    return ((1 << 2*(depth+1)) - 1)/3;
}

/// Get array index of parent node from a given node index.
inline int quadTreeParentNode(int index)
{
    return (index - 1) >> 2;
}

/// Get the array index of a quad tree node at position x,y and given depth.
inline int quadTreeNodeIndex(int x, int y, int depth)
{
    assert(depth >= 0);
    assert(x >= 0 && x < (1<<depth));
    assert(y >= 0 && y < (1<<depth));
    int index = 0;
    // Compute the offset of the node relative to the first node of the level.
    for(int i = 0, iend = 2*depth; i < iend; i+=2)
    {
        index |= ((x & 1) << i) | ((y & 1) << (i+1));
        x >>= 1;
        y >>= 1;
    }
    // Add on the offset of the first node at the level.  This is the sum of
    // the number of nodes on each previous level (which happens to be a
    // geometric series).
    return index + quadTreeNumNodes(depth-1);
}


//------------------------------------------------------------------------------
/// Data structure for geometry storage during splitting
///
/// The main design aim here is to allow the top piece of geometry to be
/// retrieved for any given bucket, independent of the order in which the
/// buckets are processed.
class SplitStore
{
    private:
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

        /// Create a storage structure with nleafx x nleafy buckets, and given
        /// spatial bound
        SplitStore(int nleafx, int nleafy, const Imath::Box2f& bound)
            : m_nodes(),
            m_nleafx(nleafx),
            m_nleafy(nleafy),
            m_treeDepth(0),
            m_bound(bound)
        {
            m_treeDepth = std::max(iceil(log2(nleafx)), iceil(log2(nleafy)));
            m_nodes.resize(quadTreeNumNodes(m_treeDepth));
        }

        /// Get identifier for leaf bucket at position x,y
        NodeId getNodeId(int x, int y) const
        {
            assert(x >= 0 && x < m_nleafx);
            assert(y >= 0 && y < m_nleafy);
            return quadTreeNodeIndex(x, y, m_treeDepth);
        }

        /// Get number of active leaf nodes in the x-direction
        int nleafx() const { return m_nleafx; }
        /// Get number of active leaf nodes in the y-direction
        int nleafy() const { return m_nleafy; }

        /// Grab the top surface for the given bucket id
        ///
        /// If there are no surfaces present in the node, return null.
        GeomHolderPtr pop(NodeId leaf)
        {
            // Search from leaf node down to the root, finding the geometry
            // closest to the camera.
            assert(leaf >= quadTreeNumNodes(m_treeDepth-1));
            assert(leaf < quadTreeNumNodes(m_treeDepth));
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
                id = quadTreeParentNode(id);
            }
            if(nodeIdToPop < 0)
                return GeomHolderPtr();
            std::vector<GeomHolderPtr>& queue = m_nodes[nodeIdToPop].queue;
            std::pop_heap(queue.begin(), queue.end(), geomHeapOrder);
            GeomHolderPtr result = queue.back();
            queue.pop_back();
            return result;
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
            // Compute tree level at which the geometry will be inserted.  We
            // choose a tree level such that the geometry is smaller than the
            // tree nodes at that level.  This ensures that the geometry is
            // inserted into at most four nodes.
            float width = m_bound.max.x - m_bound.min.x;
            float height = m_bound.max.y - m_bound.min.y;
            int xdepth = int(-log2((b.max.x - b.min.x)/width));
            int ydepth = int(-log2((b.max.y - b.min.y)/height));
            int depth = clamp(std::min(xdepth, ydepth), 0, m_treeDepth);
            // Compute integer (x,y) coordinates of the nodes which the
            // geometry bound touches at the computed insertion depth
            int x0 = (clamp(int(m_nleafx*(b.min.x - m_bound.min.x)/width),
                            0, m_nleafx-1)) >> (m_treeDepth - depth);
            int x1 = (clamp(int(m_nleafx*(b.max.x - m_bound.min.x)/width),
                            0, m_nleafx-1)) >> (m_treeDepth - depth);
            int y0 = (clamp(int(m_nleafy*(b.min.y - m_bound.min.y)/height),
                            0, m_nleafy-1)) >> (m_treeDepth - depth);
            int y1 = (clamp(int(m_nleafy*(b.max.y - m_bound.min.y)/height),
                            0, m_nleafy-1)) >> (m_treeDepth - depth);
            assert(x1-x0 <= 1);
            assert(y1-y0 <= 1);
            // Place geometry into nodes which it touches.
            for(int j = y0; j <= y1; ++j)
            {
                for(int i = x0; i <= x1; ++i)
                {
                    int index = quadTreeNodeIndex(i, j, depth);
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

// 2D BSP tree, packed array storage.
//
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

#endif // AQSIS_SPLITSTORE_H_INCLUDED

// vi:set et:
