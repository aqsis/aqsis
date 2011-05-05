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


#ifndef AQSIS_POINTCLOUD_H_INCLUDED
#define AQSIS_POINTCLOUD_H_INCLUDED

#include <cassert>
#include <cmath>
#include <vector>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImathColor.h>

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

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
        PointOctree(const PointArray& points)
            : m_root(0),
            m_dataSize(points.stride)
        {
            size_t npoints = points.size();
            // Super naive, recursive top-down construction.
            //
            // TODO: Investigate bottom-up construction based on sorting in
            // order of space filling curve.
            Box3f bound;
            std::vector<const float*> workspace(npoints);
            for(size_t i = 0; i < npoints; ++i)
            {
                const float* p = &points.data[i*m_dataSize];
                bound.extendBy(V3f(p[0], p[1], p[2]));
                workspace[i] = &points.data[i*m_dataSize];
            }
            // We make octree bound cubic rather than fitting the point cloud
            // tightly.  This improves the distribution of points in the octree
            // nodes and reduces artifacts when groups of points are aggregated
            // in the internal nodes.
            //
            // If we *don't* do this and we have a rectangular (non-cubic)
            // bound, we end up with a lot more points in one direction inside
            // a node than another.  This means the aggregated averaged point -
            // intended to represent the collection - is in the middle, but
            // with lots of room on either side:
            //
            // +-----------+   ----->    +----/^\----+
            // | o o o o o |  aggregate  |   | . |   |
            // +-----------+             +----\_/----+
            //
            //   <------->                   <--->
            // even distribution           all in middle :(
            //
            // That is, there will be large gaps between neighbouring disks,
            // which gives large transparent gaps in the microrendered surface.
            // Obviously a bad thing!
            V3f d = bound.size();
            V3f c = bound.center();
            float maxDim2 = std::max(std::max(d.x, d.y), d.z) / 2;
            bound.min = c - V3f(maxDim2);
            bound.max = c + V3f(maxDim2);
            m_root = makeTree(&workspace[0], npoints, m_dataSize, bound);
        }

        ~PointOctree()
        {
            deleteTree(m_root);
        }

        /// Get root node of tree
        const Node* root() const { return m_root; }

        /// Get size of point data
        int dataSize() const { return m_dataSize; }

    private:
        static Node* makeTree(const float** points, size_t npoints,
                              int dataSize, const Box3f& bound)
        {
            assert(npoints != 0);
            Node* node = new Node;
            node->bound = bound;
            V3f c = bound.center();
            node->center = c;
            V3f diag = bound.size();
            node->boundRadius = diag.length()/2.0f;
            node->npoints = 0;
            size_t pointsPerLeaf = 8;
            if(npoints <= pointsPerLeaf)
            {
                // Small number of child points: make this a leaf node and
                // store the points directly in the data member.
                node->npoints = npoints;
                // Copy over data into node.
                node->data.reset(new float[npoints*dataSize]);
                float sumA = 0;
                V3f sumP(0);
                V3f sumN(0);
                C3f sumCol(0);
                for(size_t j = 0; j < npoints; ++j)
                {
                    const float* p = points[j];
                    // copy extra data
                    for(int i = 0; i < dataSize; ++i)
                        node->data[j*dataSize + i] = p[i];
                    // compute averages (area weighted)
                    float A = p[6]*p[6];
                    sumA += A;
                    sumP += A*V3f(p[0], p[1], p[2]);
                    sumN += A*V3f(p[3], p[4], p[5]);
                    sumCol += A*C3f(p[7], p[8], p[9]);
                }
                node->aggP = 1.0f/sumA * sumP;
                node->aggN = sumN.normalized();
                node->aggR = sqrtf(sumA);
                node->aggCol = 1.0f/sumA * sumCol;
                return node;
            }

            // allocate extra workspace for storing child points (ugh!)
            std::vector<const float*> workspace(8*npoints);
            const float** w = &workspace[0];
            const float** P[8] = {
                w,             w + npoints,   w + 2*npoints, w + 3*npoints,
                w + 4*npoints, w + 5*npoints, w + 6*npoints, w + 7*npoints
            };
            // Partition points into the eight child nodes
            size_t np[8] = {0};
            for(size_t i = 0; i < npoints; ++i)
            {
                const float* p = points[i];
                int cellIndex = 4*(p[2] > c.z) + 2*(p[1] > c.y) + (p[0] > c.x);
                P[cellIndex][np[cellIndex]++] = p;
            }

            // Recursively generate child nodes and compute position, normal
            // and radius for the current node.
            float sumA = 0;
            V3f sumP(0);
            V3f sumN(0);
            C3f sumCol(0);
            for(int i = 0; i < 8; ++i)
            {
                if(np[i] == 0)
                    continue;
                Box3f bnd;
                bnd.min.x = (i     % 2 == 0) ? bound.min.x : c.x;
                bnd.min.y = ((i/2) % 2 == 0) ? bound.min.y : c.y;
                bnd.min.z = ((i/4) % 2 == 0) ? bound.min.z : c.z;
                bnd.max.x = (i     % 2 == 0) ? c.x : bound.max.x;
                bnd.max.y = ((i/2) % 2 == 0) ? c.y : bound.max.y;
                bnd.max.z = ((i/4) % 2 == 0) ? c.z : bound.max.z;
                Node* child = makeTree(P[i], np[i], dataSize, bnd);
                node->children[i] = child;
                // Weighted average with weight = disk surface area.
                float A = child->aggR * child->aggR;
                sumA += A;
                sumP += A * child->aggP;
                sumN += A * child->aggN;
                sumCol += A * child->aggCol;
            }
            node->aggP = 1.0f/sumA * sumP;
            node->aggN = sumN.normalized();
            node->aggR = sqrtf(sumA);
            node->aggCol = 1.0f/sumA * sumCol;

            return node;
        }

        /// Recursively delete tree, depth first.
        static void deleteTree(Node* n)
        {
            if(!n) return;
            for(int i = 0; i < 8; ++i)
                deleteTree(n->children[i]);
            delete n;
        }

        Node* m_root;
        int m_dataSize;
};



//------------------------------------------------------------------------------
// Musings on abstraction for point array class... not sure what the best
// abstraction is yet.

#if 0

/// Reference to data for a single surface element
class Point
{
    public:
        Point(const float* data) : m_data(data) {}
        /// Get point position
        V3f P() const { return V3f(m_data[0], m_data[1], m_data[2]); }
        /// Get normal
        V3f N() const { return V3f(m_data[3], m_data[4], m_data[5]); }
        /// Get radius
        float r() const { return m_data[6]; }
        /// Get any additional attached data
        const float* userData() const { return &m_data[7]; }
    private:
        const float* m_data;
};


/// Array of surface elements ("points")
class PointArray
{
    public:
        PointArray(int stride)
            : m_stride(stride)
        { }

        const Point operator[](size_t i) const
        {
            assert(i < data.size());
            return &data[i*m_stride];
        }

        int stride() const { return m_stride; }

        void addPoint(const float* data)
        {
            m_data.insert(m_data.end(), data, data + m_stride);
        }

        // Get centroid of the point cloud.
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

    private:
//        struct Channel
//        {
//            int index;
//            int size;
//            std::string name;
//        };

        int m_stride;
//        std::vector<Channel> m_channelData;
        std::vector<float> m_data;
};
#endif


#endif // AQSIS_POINTCLOUD_H_INCLUDED

// vi: set et:
