// Aqsis
// Copyright (C) 1997 - 2002, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
 * \brief Hierarchical occlusion culling tree implementation
 * \author Paul Gregory
 * \author Chris Foster
 */

#include "occlusion.h"

#if _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <aqsis/util/autobuffer.h>
#include "bound.h"
#include "bucketprocessor.h"
#include "imagepixel.h"

namespace Aqsis {

//----------------------------------------------------------------------
// CqOcclusionTree implementation.


CqOcclusionTree::CqOcclusionTree()
	: m_treeBoundMin(),
	m_treeBoundMax(),
	m_depthTree(),
	m_firstLeafNode(0),
	m_numLevels(0),
	m_splitXFirst(true),
	m_needsUpdate(false)
{}

void CqOcclusionTree::setupTree(CqBucketProcessor& bp)
{
	CqRegion reg = bp.SampleRegion();
	// Now setup the new array based tree.
	// First work out how deep the tree needs to be.
	TqInt xSamples = bp.optCache().xSamps;
	TqInt ySamples = bp.optCache().ySamps;
	TqInt numXSubpix = reg.width()*xSamples;
	TqInt numYSubpix = reg.height()*ySamples;
	// Get number of subdivisions required in x and y to cover all samples.
	TqInt depthX = lceil(log2(numXSubpix));
	TqInt depthY = lceil(log2(numYSubpix));
	// Adjust the depths in the x or y directions to make sure that they don't
	// differ by more than one since we always split the tree in alternate
	// directions.
	if(depthX < depthY)
		depthX = depthY - 1;
	else if(depthY < depthX)
		depthY = depthX - 1;
	m_splitXFirst = depthX >= depthY;
	TqInt depth = depthX + depthY;

	// Depth is the number of gaps between the levels, which is one less than
	// the number of levels.
	m_numLevels = depth + 1;
	TqInt numLeafNodes = 1 << depth; // pow(2,depth)
	TqInt numTotalNodes = 2*numLeafNodes - 1;
	m_firstLeafNode = numLeafNodes - 1;
	m_depthTree.assign(numTotalNodes, 0);

	// Compute and cache bounds of tree area.
	m_treeBoundMin = CqVector2D(reg.xMin(), reg.yMin());
	CqVector2D treeDiag = compMul(reg.diagonal(),
		CqVector2D(TqFloat(1<<depthX)/numXSubpix, TqFloat(1<<depthY)/numYSubpix));
	m_treeBoundMax = m_treeBoundMin + treeDiag;

	// Now associate sample points to the leaf nodes, and initialise the leaf
	// node depths of those that contain sample points to infinity.
	for(CqSampleIterator sample = bp.pixels(reg); sample.inRegion(); ++sample)
	{
		// Compute subpixel coordinates of the sample with the top-left of the
		// bucket as origin and locate leaf node for the sample position.
		TqInt sampleNodeIndex = treeIndexForPoint(m_numLevels, m_splitXFirst,
				sample.subPixelX() - xSamples*reg.xMin(),
				sample.subPixelY() - ySamples*reg.yMin());
		// Check that the index is a leaf
		assert(sampleNodeIndex >= m_firstLeafNode && sampleNodeIndex < numTotalNodes);
		sample->occlusionIndex = sampleNodeIndex;
		assert(m_depthTree[sampleNodeIndex] == 0);
		m_depthTree[sampleNodeIndex] = FLT_MAX;
	}
	// Fix up parent depths.
	propagateDepths();
}


void CqOcclusionTree::setSampleDepth(TqFloat depth, TqInt index)
{
	assert(m_depthTree[index] >= depth);
	m_depthTree[index] = depth;
	m_needsUpdate = true;
}

void CqOcclusionTree::updateTree()
{
	// Only update the depths if the leaf nodes have changed since the last
	// update.
	if(m_needsUpdate)
		propagateDepths();
}

/** \brief Propagate depths from leaf nodes up the tree to the root.
 *
 * This ensures that the depths stored in the tree are valid, given
 * that the leaf nodes are up to date.
 */
void CqOcclusionTree::propagateDepths()
{
	// Iterate over each level of the tree in turn, starting at one
	// level below the leaf nodes, and ending at the root.  This
	// algorithm is cache-coherent.
	for(int i = static_cast<int>(std::pow(2.0, m_numLevels-1)) - 2; i >= 0; --i)
		m_depthTree[i] = max(m_depthTree[2*i+1], m_depthTree[2*i+2]);
	m_needsUpdate = false;
}


namespace {

/// Helper struct for tree traversal representing an area of one of the nodes.
struct SqNodeBound
{
	SqNodeBound()
		: minX(0), minY(0), maxX(0), maxY(0), index(0), splitInX(true)
	{}
	SqNodeBound(TqFloat minX, TqFloat minY, TqFloat maxX, TqFloat maxY, TqInt index, bool splitInX)
		: minX(minX), minY(minY), maxX(maxX), maxY(maxY), index(index), splitInX(splitInX)
	{}
	/// Minimum extent of bounding box
	TqFloat minX, minY;
	TqFloat maxX, maxY;
	TqInt index;
	bool splitInX;
};

} // unnamed namespace

bool CqOcclusionTree::canCull(const CqBound& bound) const
{
	// Crop the input bound to the culling bound.
	TqFloat tminX = max(bound.vecMin().x(), m_treeBoundMin.x());
	TqFloat tminY = max(bound.vecMin().y(), m_treeBoundMin.y());
	TqFloat tmaxX = min(bound.vecMax().x(), m_treeBoundMax.x());
	TqFloat tmaxY = min(bound.vecMax().y(), m_treeBoundMax.y());

	// Auto buffer with enough auto-allocated room for a stack which can
	// traverse a depth-20 tree (2^20 = 1024 * 1024 samples in a bucket == lots :)
	CqAutoBuffer<SqNodeBound, 40> stack(2*m_numLevels);
	TqInt top = -1;
	// The root node of tree covers the whole bound of the bucket.
	stack[++top] = SqNodeBound(m_treeBoundMin.x(), m_treeBoundMin.y(),
			m_treeBoundMax.x(), m_treeBoundMax.y(), 0, m_splitXFirst);

	// Traverse the tree, starting at the root.  We want to find if any of the
	// leaf nodes are further away than the bound.
	while(top >= 0)
	{
		SqNodeBound node = stack[top--];

		// If the bound intersects the node.
		if( !(tminX > node.maxX || tminY > node.maxY ||
			  tmaxX < node.minX || tmaxY < node.minY) )
		{
			// If the depth stored at the node is closer than the minimum Z of
			// the bound, this node won't cause the surface to be rendered.
			if(m_depthTree[node.index] < bound.vecMin().z()) 
				continue;

			// If the node is a leaf node and is behind the bound, the surface
			// cannot be culled and we can return.
			if(node.index >= m_firstLeafNode)
				return false;

			// Otherwise split the current bound across remaining nodes.
			if(node.splitInX)
			{
				TqFloat avgX = 0.5 * (node.maxX + node.minX);
				stack[++top] = SqNodeBound(node.minX, node.minY, avgX, node.maxY, node.index * 2 + 1, !node.splitInX);
				stack[++top] = SqNodeBound(avgX, node.minY, node.maxX, node.maxY, node.index * 2 + 2, !node.splitInX);
			}
			else
			{
				TqFloat avgY = 0.5*(node.maxY + node.minY);
				stack[++top] = SqNodeBound(node.minX, node.minY, node.maxX, avgY, node.index * 2 + 1, !node.splitInX);
				stack[++top] = SqNodeBound(node.minX, avgY, node.maxX, node.maxY, node.index * 2 + 2, !node.splitInX);
			}
		}
	}
	return true;
}


/** \brief Return the tree index for leaf node containing the given point.
 *
 * Here is an example binary spatial subdivision, which may be represented as a
 * binary tree.  The array indices in which leaf nodes should be stored are
 * shown.
 *
 * \verbatim
 *
 * +---------------+-0.0f
 * |   |   |   |   |  |
 * | 7 | 8 | 11| 12|  |       0
 * |   |   |   |   |  |
 * |-------|-------|  |      integer y-coords of leaf
 * |   |   |   |   |  |
 * | 9 | 10| 13| 14|  |       1
 * |   |   |   |   |  v
 * +---------------+-1.0f
 * |               |
 * 0.0f  ----->    1.0f
 *
 * integer x-coords of leaf
 *   0   1   2   3
 *
 * \endverbatim
 *
 *
 * \param numLevels - number of levels in the tree, (just the root node has
 *                    numLevels == 1).
 * \param (x,y) - coordinates of sample within the tree, counting from (0,0) in
 *                the top left.
 *
 * \return An index for a 2D binary tree stored in a 0-based array.
 */
TqInt CqOcclusionTree::treeIndexForPoint(TqInt numLevels, bool splitXFirst,
		TqInt x, TqInt y)
{
	assert(numLevels > 0);
	// The coordinates must lie inside the tree bounds.
	assert(x < (1 << (numLevels-!splitXFirst/2)));
	assert(y < (1 << ((numLevels-splitXFirst)/2)));

	// The LSB should come from the coordinate which is split last before the
	// leaves.  Swap here ensure that.
	if(!splitXFirst ^ (numLevels % 2 == 1))
		std::swap(x,y);

	// This is the base index for the first leaf in a tree with numLevels
	// levels, stored in a 1-based array.
	TqInt index = 1 << (numLevels-1);
	// Interlace the bits of x and y to form the index.  Assuming that x has
	// the last split before the leaves, every second bit of the index starting
	// with the LSB should come from x.  Every other bit starting from the
	// LSB+1 should come from y.
	TqInt idxBit = 0;
	while(x != 0 || y != 0)
	{
		index |= (x & 1) <<  idxBit
			   | (y & 1) << (idxBit+1);
		x >>= 1;
		y >>= 1;
		idxBit += 2;
	}
	// Finally subtract one from the index to get to the location in a
	// zero-based array.
	return index - 1;
}

} // namespace Aqsis

