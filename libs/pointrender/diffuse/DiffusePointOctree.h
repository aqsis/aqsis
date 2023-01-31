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


#ifndef DIFFUSEPOINTOCTREE_H_
#define DIFFUSEPOINTOCTREE_H_

#include <vector>

#include <Imath/ImathVec.h>
#include <Imath/ImathBox.h>
#include <Imath/ImathColor.h>

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include "PointArray.h"

namespace Aqsis {

/**
 * Load in point array from aqsis point cloud file format.
 *
 * The point cloud file must at a minimum include a float attribute "_area".
 * The position, normal, area and optionally radiosity ("_radiosity") will be
 * loaded into the PointArray which is returned.  The points will be
 * _appended_ to the provided points PointArray.  The return value is true on
 * success, false on error.
 *
 * @param points
 * @param fileName
 * @return
 */
bool loadDiffusePointFile(PointArray& points, const std::string& fileName);


/**
 * This class offers a naive way of storing diffuse surfels in a point hierarchy.
 */
class DiffusePointOctree {

public:

	/**
	 * This struct is a node in the hierarchy of the surfels.
	 *
	 * Leaf nodes have npoints > 0, specifying the number of child points
	 * contained.
	 */
	struct Node {
		Node() :
			bound(), center(0), boundRadius(0), aggP(0), aggN(0), aggR(0),
					aggCol(0), npoints(0), data() {
			children[0] = children[1] = children[2] = children[3] = 0;
			children[4] = children[5] = children[6] = children[7] = 0;
		}

		/// Data derived from octree bounding box
		Imath::Box3f bound;
		Imath::V3f center;
		float boundRadius;
		// Crude aggregate values for position, normal and radius
		Imath::V3f aggP;
		Imath::V3f aggN;
		float aggR;
		Imath::C3f aggCol;
		// Child nodes, to be indexed as children[z][y][x]
		Node* children[8];
		// bool used;
		/// Number of child points for the leaf node case
		int npoints;
		// Collection of points in leaf.
		boost::scoped_array<float> data;
	};


private:

	Node* m_root; //< The root node of the diffuse surfel hierarchy.
	int m_dataSize; // The size of each surfel/point (in floats).

public:

	/**
	 * Construct an octree hierarchy of diffuse surfels/points from an
	 * array of points.
	 *
	 * @param points
	 * 			The array of points.
	 */
	DiffusePointOctree(const PointArray& points);

	/**
	 * Get the root node of tree of this octree.
	 *
	 * @return The root node of tree of this octree.
	 */
	const Node* root() const {
		return m_root;
	}

	/**
	 * Get the number of floats representing each point/surfel.
	 *
	 * @return
	 * 		The number of floats representing each point/surfel.
	 */
	int dataSize() const {
		return m_dataSize;
	}

	/**
	 * Destructor of the octree hierarchy of diffuse surfels.
	 */
	~DiffusePointOctree();

private:

	/**
	 * Build an octree node from the given points
	 *
	 * @param depth
	 * 			The depth of the node to be created
	 * @param points
	 * 			A pointer to the point data for this node.
	 * @param npoints
	 * 			The number of points that should be included by this node.
	 * @param dataSize
	 * 			The number of floats representing each point.
	 * @param bound
	 * 			The bounding box that encloses all the points to be included.
	 * @return
	 * 			An octree node including all passed points.
	 */
	static Node* makeTree(int depth, const float** points, size_t npoints,
			int dataSize, const Imath::Box3f& bound);

	/**
	 * Recursively delete the octree, depth first.
	 *
	 * @param n
	 * 		The node to delete.
	 */
	static void deleteTree(Node* n);

};

}
#endif /* DIFFUSEPOINTOCTREE_H_ */
