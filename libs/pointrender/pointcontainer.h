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


#ifndef AQSIS_POINTCONTAINER_H_INCLUDED
#define AQSIS_POINTCONTAINER_H_INCLUDED

#include <map>
#include <vector>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImathColor.h>

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>


namespace Aqsis {

using Imath::V3f;
using Imath::Box3f;
using Imath::C3f;

/// Array of surface elements
///
/// TODO: Make into a class, etc.
///
/// Point data is stored in a flat array as
///
///     [P1 N1 r1 user1  P2 N2 r2 user2  ... ]
///
/// where user1 user2... is extra "user data" appended after the position
/// normal, and radius data.
///
struct PointArray
{
    int stride;
    std::vector<float> data;

    /// Get number of points in cloud
    size_t size() const { return data.size()/stride; }

    /// Get centroid of point cloud.
    V3f centroid() const
    {
        V3f sum(0);
        for(std::vector<float>::const_iterator p = data.begin();
            p < data.end(); p += stride)
        {
            sum += V3f(p[0], p[1], p[2]);
        }
        return (1.0f/data.size()*stride) * sum;
    }
};


/// Load in point array from aqsis point cloud file format
///
/// The point cloud file must at a minimum include a float attribute "_area".
/// The position, normal, area and optionally radiosity ("_radiosity") will be
/// loaded into the PointArray which is returned.  The points will be
/// _appended_ to the provided points PointArray.  The return value is true on
/// success, false on error.
bool loadPointFile(PointArray& points, const std::string& fileName);


//------------------------------------------------------------------------------
/// Naive octree for storing a point hierarchy
class PointOctree
{
    public:
        /// Tree node
        ///
        /// Leaf nodes have npoints > 0, specifying the number of child points
        /// contained.
        struct Node
        {
            Node()
                : bound(),
                center(0),
                boundRadius(0),
                aggP(0),
                aggN(0),
                aggR(0),
                aggCol(0),
                npoints(0),
                data()
            {
                children[0] = children[1] = children[2] = children[3] = 0;
                children[4] = children[5] = children[6] = children[7] = 0;
            }

            /// Data derived from octree bounding box
            Box3f bound;
            V3f center;
            float boundRadius;
            // Crude aggregate values for position, normal and radius
            V3f aggP;
            V3f aggN;
            float aggR;
            C3f aggCol;
            // Child nodes, to be indexed as children[z][y][x]
            Node* children[8];
            // bool used;
            /// Number of child points for the leaf node case
            int npoints;
            // Collection of points in leaf.
            boost::scoped_array<float> data;
        };

        /// Construct tree from array of points.
        PointOctree(const PointArray& points);

        ~PointOctree();

        /// Get root node of tree
        const Node* root() const { return m_root; }

        /// Get number of floats representing each point.
        int dataSize() const { return m_dataSize; }

    private:
        /// Build a tree node from the given points
        ///
        /// \param depth - depth of the node to be created
        /// \param points - pointers to point data
        /// \param npoints - number of points in points array
        /// \param dataSize - number of floats representing each point
        static Node* makeTree(int depth, const float** points, size_t npoints,
                              int dataSize, const Box3f& bound);

        /// Recursively delete tree, depth first.
        static void deleteTree(Node* n);

        Node* m_root;
        int m_dataSize;
};



//------------------------------------------------------------------------------
/// Cache for previously loaded point clouds
class PointOctreeCache
{
    public:
        /// Find a cached point octree by file name
        ///
        /// Returns a null pointer if the file couldn't be found or opened.
        ///
        /// TODO: Search path handling.
        const PointOctree* find(const std::string& fileName);

        /// Clear all trees from the cache
        void clear();

    private:
        typedef std::map<std::string, boost::shared_ptr<PointOctree> > MapType;
        MapType m_cache;
};



} // namespace Aqsis

#endif // AQSIS_POINTCONTAINER_H_INCLUDED

// vi: set et:
