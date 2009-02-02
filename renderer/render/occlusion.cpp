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

#include "autobuffer.h"
#include "bound.h"
#include "bucketprocessor.h"
#include "imagepixel.h"

namespace Aqsis {

/// Global occlusion tree
/// \todo Remove!
CqOcclusionTree g_occlusionTree;

//----------------------------------------------------------------------
// CqOcclusionTree implementation.

CqOcclusionTree::CqOcclusionTree()
	: m_treeBoundMin(),
	m_treeBoundMax(),
	m_cullBoundMin(),
	m_cullBoundMax(),
	m_leafSamples(),
	m_depthTree(),
	m_firstLeafNode(0),
	m_numLevels(0)
{}

void CqOcclusionTree::setupTree(const CqBucketProcessor* bp, TqInt xMin, TqInt yMin,
				TqInt xMax, TqInt yMax)
{
	// Now setup the new array based tree.
	// First work out how deep the tree needs to be.
	TqInt numSamples = bp->numSamples();
	TqInt depth = lceil(log2(numSamples));
	m_numLevels = depth + 1;
	TqInt numTotalNodes = lceil(std::pow(2.0, depth+1))-1;
	TqInt numLeafNodes = lceil(std::pow(2.0, depth));
	m_firstLeafNode = numLeafNodes - 1;
	m_depthTree.assign(numTotalNodes, 0);
	m_leafSamples.assign(numLeafNodes, std::vector<const SqSampleData*>());

	// Compute and cache bounds of tree and culling area.
	m_treeBoundMin = CqVector2D(bp->SampleRegion().xMin(), bp->SampleRegion().yMin());
	CqVector2D treeDiag(bp->SampleRegion().diagonal());
	m_treeBoundMax = m_treeBoundMin + treeDiag;
	m_cullBoundMin = CqVector2D(xMin, yMin);
	m_cullBoundMax = CqVector2D(xMax, yMax);

	// Now associate sample points to the leaf nodes, and initialise the leaf
	// node depths of those that contain sample points to infinity.
	const std::vector<SqSampleData>& samples = bp->SamplePoints();
	std::vector<SqSampleData>::const_iterator sample;
	for(sample = samples.begin(); sample != samples.end(); ++sample)
	{
		// Convert samplePos into normalized units for finding sample position.
		CqVector2D samplePos = sample->m_Position - m_treeBoundMin;
		samplePos.x(samplePos.x() / treeDiag.x());
		samplePos.y(samplePos.y() / treeDiag.y());
		// Locate leaf node for the sample position
		TqInt sampleNodeIndex = treeIndexForPoint(depth+1, samplePos);
		// Check that the index is within the tree
		assert(sampleNodeIndex < numTotalNodes);
		// Check that the index is a leaf node.
		assert((sampleNodeIndex*2)+1 >= numTotalNodes);

		m_depthTree[sampleNodeIndex] = FLT_MAX;
		m_leafSamples[sampleNodeIndex-m_firstLeafNode].push_back(&(*sample));
	}
	// Fix up parent depths.
	propagateDepths();
}

void CqOcclusionTree::updateDepths()
{
	if(m_depthTree.size() == 0)
		return;

	// Set the terminal node depths to the furthest of the sample points they contain.
	for(TqInt i = 0, numLeafNodes = m_leafSamples.size(); i < numLeafNodes; ++i)
	{
		if(m_leafSamples[i].size() == 0)
			continue;
		TqFloat max = 0;
		bool hit = false;
		for(std::vector<const SqSampleData*>::iterator sample = m_leafSamples[i].begin(),
				end = m_leafSamples[i].end(); sample != end; ++sample)
		{
			TqFloat depth; 
			if((*sample)->m_OpaqueSample.isValid() &&
				(depth = (*sample)->m_OpaqueSample.Data()[Sample_Depth]) > max)
			{
				max = depth;
				hit = true;
			}
		}
		m_depthTree[i + m_firstLeafNode] = hit ? max : FLT_MAX;
	}
	// Propagate the changes up the tree.
	propagateDepths();
}

namespace {

/// Helper struct for tree traversal representing an area of one of the nodes.
struct SqNodeStack
{
	SqNodeStack()
		: minX(0), minY(0), maxX(0), maxY(0), index(0), splitInX(true)
	{}
	SqNodeStack(TqFloat minX, TqFloat minY, TqFloat maxX, TqFloat maxY, TqInt index, bool splitInX)
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
	TqFloat tminX = max(bound.vecMin().x(), m_cullBoundMin.x());
	TqFloat tminY = max(bound.vecMin().y(), m_cullBoundMin.y());
	TqFloat tmaxX = min(bound.vecMax().x(), m_cullBoundMax.x());
	TqFloat tmaxY = min(bound.vecMax().y(), m_cullBoundMax.y());

	// Auto buffer with enough auto-allocated room for a stack which can
	// traverse a depth-20 tree (2^20 = 1024 * 1024 samples in a bucket == lots :)
	CqAutoBuffer<SqNodeStack, 40> stack(2*m_numLevels);
	TqInt top = -1;
	// The root node of tree covers the whole bound of the bucket.
	stack[++top] = SqNodeStack(m_treeBoundMin.x(), m_treeBoundMin.y(),
			m_treeBoundMax.x(), m_treeBoundMax.y(), 0, true);
	SqNodeStack terminatingNode(0.0f, 0.0f, 0.0f, 0.0f, 0, true);

	// Traverse the tree, starting at the root.  We want to find if any of the
	// leaf nodes are further away than the bound.
	while(top >= 0)
	{
		SqNodeStack node = stack[top--];

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
				stack[++top] = SqNodeStack(node.minX, node.minY, avgX, node.maxY, node.index * 2 + 1, !node.splitInX);
				stack[++top] = SqNodeStack(avgX, node.minY, node.maxX, node.maxY, node.index * 2 + 2, !node.splitInX);
			}
			else
			{
				TqFloat avgY = 0.5*(node.maxY + node.minY);
				stack[++top] = SqNodeStack(node.minX, node.minY, node.maxX, avgY, node.index * 2 + 1, !node.splitInX);
				stack[++top] = SqNodeStack(node.minX, avgY, node.maxX, node.maxY, node.index * 2 + 2, !node.splitInX);
			}
		}
	}
	return true;
}


/** \brief Return the tree index for leaf node containing the given point.
 *
 * \param treeDepth - depth of the tree, (root node has treeDepth == 1).
 *
 * \param p - A point which we'd like the index for.  We assume that the
 *            possible points lie in the box [0,1) x [0,1)
 *
 * \return An index for a 2D binary tree stored in a 0-based array.
 */
TqInt CqOcclusionTree::treeIndexForPoint(TqInt treeDepth, const CqVector2D& p)
{
	assert(treeDepth > 0);
	//assert(p.x() >= 0 && p.x() <= 1);
	//assert(p.y() >= 0 && p.y() <= 1);

	const TqInt numXSubdivisions = treeDepth / 2;
	const TqInt numYSubdivisions = (treeDepth-1) / 2;
	// true if the last subdivison was along the x-direction.
	const bool lastSubdivisionInX = (treeDepth % 2 == 0);

	// integer coordinates of the point in terms of the subdivison which it
	// falls into, staring from 0 in the top left.
	TqInt x = static_cast<int>(floor(p.x() * (1 << numXSubdivisions)));
	TqInt y = static_cast<int>(floor(p.y() * (1 << numYSubdivisions)));

	// This is the base coordinate for the first leaf in a tree of depth "treeDepth".
	TqInt index = 1 << (treeDepth-1);
	if(lastSubdivisionInX)
	{
		// Every second bit of the index (starting with the LSB) should make up
		// the coordinate x, so distribute x into index in that fashion.
		//
		// Similarly for y, except alternating with the bits of x (starting
		// from the bit up from the LSB.)
		for(TqInt i = 0; i < numXSubdivisions; ++i)
		{
			index |= (x & (1 << i)) << i
				| (y & (1 << i)) << (i+1);
		}
	}
	else
	{
		// This is the opposite of the above: x and y are interlaced as before,
		// but now the LSB of y rather than x goes into the LSB of index.
		for(TqInt i = 0; i < numYSubdivisions; ++i)
		{
			index |= (y & (1 << i)) << i
				| (x & (1 << i)) << (i+1);
		}
	}
	return index - 1;
}

/** \brief Propagate depths from leaf nodes up the tree to the root.
 *
 * This ensures that the depths stored in the tree are valid, given that the
 * leaf nodes are up to date.
 */
void CqOcclusionTree::propagateDepths()
{
	// Iterate over each level of the tree in turn, starting at one
	// level below the leaf nodes, and ending at the root.  This
	// algorithm is cache-coherent.
	for(int i = static_cast<int>(std::pow(2.0, m_numLevels-1)) - 2; i >= 0; --i)
		m_depthTree[i] = max(m_depthTree[2*i+1], m_depthTree[2*i+2]);
}

} // namespace Aqsis

