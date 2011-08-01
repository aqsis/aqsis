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

/// \file Occlusion tree
/// \author Chris Foster

#ifndef AQSIS_OCCLUSION_H_INCLUDED
#define AQSIS_OCCLUSION_H_INCLUDED

#include <cfloat>

#include "treearraystorage.h"
#include "util.h"

namespace Aqsis {

/// Balanced quad tree of sample depths.
///
/// The basic task is that we have a 2D array of sample points, and we want to
/// determine whether a given 3D bound lies behind all those samples.  If so,
/// the object corresponding to the bound must be hidden and can be culled.
///
/// Choosing a good algorithm is tricky since occlusion performance depends
/// strongly on the type of scene being rendered.  On the one hand, we can have
/// very efficient occlusion queries using a tree data structure, but such
/// trees are expensive to keep up to date.  On the other hand, we can have
/// slow occlusion queries without a tree, but avoid keeping the tree up to
/// date.
///
/// There's at least two regimes of interest:
///
/// 1) Simple depth complexity.  In this case (up to a few layers of occluded
/// surfaces, perhaps 10 or somewhat more), the fastest algorithm appears to be
/// using a plain old z-buffer.  We perform occlusion culling of a bound by
/// checking each sample depth inside the bound.
///
/// 2) Strong depth complexity.  In this case, it's effective to have access to
/// a tree data structure where interior nodes store the max depths of nodes
/// below them.  This is more or less the algorithm presented in the paper
/// "Hierarchial Z-Buffer Visibility" by Greene, Kass and Miller, 1993.  The
/// upshot is very cheap occlusion queries for most surfaces, sometimes as
/// simple as a single comparison with the root node depth.
///
/// Note that the simple approach to occlusion here degrades in quality in the
/// presence of motion blur and depth of field.  The paper "Space-Time
/// Hierarchical Occlusion Culling for Micropolygon Rendering with Motion Blur"
/// by Boulos et al. is probably worth checking out in this regard.
///
class OcclusionTree
{
    public:
        OcclusionTree(V2i nleaves)
            : m_nleaves(nleaves),
            m_treeDepth(iceil(std::max(log2(nleaves.x), log2(nleaves.y)))),
            m_nleavesFull(1 << m_treeDepth),
            m_z(new float[quadTreeNumNodes(m_treeDepth)])
        { }

        /// Reset the occlusion tree so that leaf nodes are at infinity.
        void reset()
        {
            // Leaf nodes at 0 or infinity, depending on whether they're in
            // the range or not.  Setting the ones outside the range to 0
            // prevents them from interfering with the tree updates.
            for(int j = 0; j < m_nleavesFull; ++j)
            for(int i = 0; i < m_nleavesFull; ++i)
            {
                if(i < m_nleaves.x && j < m_nleaves.y)
                    m_z[quadTreeNodeIndex(i, j, m_treeDepth)] = FLT_MAX;
                else
                    m_z[quadTreeNodeIndex(i, j, m_treeDepth)] = 0;
            }
            updateDepths();
        }

        /// Determine whether an object is fully occluded.
        ///
        /// bound - object bounding box
        /// zmin - minimum depth of the object
        bool isOccluded(Box2i bound, float zmin)
        {
            // clamp geometry bound to extent of sample region
            bound.min = V2i(clamp(bound.min.x, 0, m_nleaves.x),
                            clamp(bound.min.y, 0, m_nleaves.y));
            bound.max = V2i(clamp(bound.max.x, 0, m_nleaves.x),
                            clamp(bound.max.y, 0, m_nleaves.y));
            Box2i rootBound(V2i(0), V2i(m_nleavesFull));
            return isOccluded(bound, zmin, rootBound, 0, 0);
        }

        /// Get occlusion tree index of the leaf node at (x,y)
        ///
        /// This is for caching purposes, so that the nodes can be quickly
        /// updated.
        int nodeIndex(int x, int y) const
        {
            return quadTreeNodeIndex(x, y, m_treeDepth);
        }

        /// Get depth at given node index.
        ///
        /// This may be used to determine the current occluding depth for a
        /// sample.  Use nodeIndex() to get the appropriate index from spatial
        /// position.
        float getDepth(int nodeIdx) const
        {
            return m_z[nodeIdx];
        }

        /// Set the depth of the given leaf node.
        ///
        /// This should be called to update the leaf nodes at a given depth.
        ///
        /// TODO: Figure out a more efficient way to do these updates, perhaps
        /// deferred subtree updates?
        void setDepth(int nodeIdx, float z)
        {
            int i = nodeIdx;
            m_z[i] = z;
            {
                // Unroll once for a little extra speed.
                assert(i > 0);
                i = quadTreeParentNode(i);
                float znew = std::max(std::max(m_z[4*i+1], m_z[4*i+2]),
                                      std::max(m_z[4*i+3], m_z[4*i+4]));
                if(znew == m_z[i])
                    return;
                m_z[i] = znew;
            }
            while(i > 0)
            {
                i = quadTreeParentNode(i);
                float znew = std::max(std::max(m_z[4*i+1], m_z[4*i+2]),
                                      std::max(m_z[4*i+3], m_z[4*i+4]));
                if(znew == m_z[i])
                    return;
                m_z[i] = znew;
            }
        }

    private:
        /// Build the tree from the leaf nodes up to the root.
        ///
        /// This function updates the interior nodes of the tree, assuming that
        /// the leaf nodes have been set to valid values.
        void updateDepths()
        {
            // We update internal nodes of the tree using a sweep starting at
            // leaf nodes and going up to the root.  This should be nice and
            // cache coherent since the access patterns are very simple.
            for(int i = quadTreeNumNodes(m_treeDepth-1)-1; i >= 0; --i)
                m_z[i] = std::max(std::max(m_z[4*i+1], m_z[4*i+2]),
                                  std::max(m_z[4*i+3], m_z[4*i+4]));
        }

        /// Determine whether a given node is occluded.
        bool isOccluded(const Box2i& geomBound, float geomZ,
                        const Box2i& nodeBound, int nodeIndex,
                        int nodeDepth)
        {
            // If geometry bound is outside the current node bound, just
            // ignore it.
            if(geomBound.min.x >= nodeBound.max.x ||
               geomBound.max.x <= nodeBound.min.x ||
               geomBound.min.y >= nodeBound.max.y ||
               geomBound.max.y <= nodeBound.min.y)
                return true;
            // If we've reached a leaf node, compare to current depth
            if(nodeDepth == m_treeDepth)
                return geomZ > m_z[nodeIndex];
            // We're in an interior node, can return if the bound is
            // definitely occluded.
            if(geomZ > m_z[nodeIndex])
                return true;
            // descend into sub levels
            int midx = (nodeBound.max.x + nodeBound.min.x) >> 1;
            int midy = (nodeBound.max.y + nodeBound.min.y) >> 1;

            // Bounds for the four child nodes.
            // For child node ordering, see treearraystorage.h
            Box2i bound1(nodeBound.min, V2i(midx,midy));
            Box2i bound2(V2i(midx, nodeBound.min.y),
                                V2i(nodeBound.max.x, midy));
            Box2i bound3(V2i(nodeBound.min.x, midy),
                                V2i(midx,nodeBound.max.y));
            Box2i bound4(V2i(midx,midy), nodeBound.max);

            return isOccluded(geomBound, geomZ, bound1,
                              4*nodeIndex + 1, nodeDepth + 1) &&
                   isOccluded(geomBound, geomZ, bound2,
                              4*nodeIndex + 2, nodeDepth + 1) &&
                   isOccluded(geomBound, geomZ, bound3,
                              4*nodeIndex + 3, nodeDepth + 1) &&
                   isOccluded(geomBound, geomZ, bound4,
                              4*nodeIndex + 4, nodeDepth + 1);
        }

        /// Debugging tool - print out the tree.
        /*
        void printTree()
        {
            for(int d = 0; d <= m_treeDepth; ++d)
            {
                int levelSize = 1<<d;
                for(int j = 0; j < levelSize; ++j)
                {
                    for(int i = 0; i < levelSize; ++i)
                        std::cout << m_z[quadTreeNodeIndex(i, j, d)] << " ";
                    std::cout << "\n";
                }
                std::cout << "\n";
            }
        }
        */

        V2i m_nleaves;                  ///< number of leaf nodes in x and y directions
        int m_treeDepth;                ///< number of levels in tree
        int m_nleavesFull;              ///< full tree leaf node number in x or y direction
        boost::scoped_array<float> m_z; ///< array-based quadtree of z values
};


} // namespace Aqsis

#endif // AQSIS_OCCLUSION_H_INCLUDED
