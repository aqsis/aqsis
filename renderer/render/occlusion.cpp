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

//----------------------------------------------------------------------
// CqOcclusionTree implementation.

CqOcclusionTree::CqOcclusionTree()
	: m_treeBoundMin(),
	m_treeBoundMax(),
	m_depthTree(),
	m_leafDepthLists(),
	m_firstLeafNode(0),
	m_numLevels(0),
	m_needsUpdate(false)
{}

void CqOcclusionTree::setupTree(CqBucketProcessor& bp)
{
	// Now setup the new array based tree.
	// First work out how deep the tree needs to be.
	TqInt numSamples = bp.numSamples();
	TqInt depth = lceil(log2(numSamples));
	m_numLevels = depth + 1;
	TqInt numLeafNodes = 1 << depth; // pow(2,depth)
	TqInt numTotalNodes = 2*numLeafNodes - 1;
	m_firstLeafNode = numLeafNodes - 1;
	m_depthTree.assign(numTotalNodes, 0);
	m_leafDepthLists.assign(numTotalNodes, std::vector<TqFloat>());

	// Compute and cache bounds of tree and culling area.
	m_treeBoundMin = CqVector2D(bp.SampleRegion().xMin(), bp.SampleRegion().yMin());
	m_treeBoundMax = CqVector2D(bp.SampleRegion().xMax(), bp.SampleRegion().yMax());

	CqVector2D treeDiag = m_treeBoundMax - m_treeBoundMin;
	// Now associate sample points to the leaf nodes, and initialise the leaf
	// node depths of those that contain sample points to infinity.
	std::vector<CqImagePixelPtr>& pixels = bp.pixels();
	std::vector<bool> leafOccupied(numLeafNodes, false);
	for(std::vector<CqImagePixelPtr>::iterator p = pixels.begin(),
			e = pixels.end(); p != e; ++p)
	{
		CqImagePixel& pixel = **p;
		for(int i = 0, numSamples = pixel.numSamples(); i < numSamples; ++i)
		{
			// Convert samplePos into normalized units for finding sample position.
			CqVector2D samplePos = pixel.SampleData(i).position - m_treeBoundMin;
			samplePos /= treeDiag;
			if(samplePos.x() < 0 || samplePos.x() > 1
				|| samplePos.y() < 0 || samplePos.y() > 1)
			{
				// Ignore samples which are outside the bucket sample region;
				// any surfaces hitting such samples must have been donated
				// from a neighbouring bucket and we may cull surfaces which
				// touch them.
				continue;
			}
			// Locate leaf node for the sample position
			TqInt sampleNodeIndex = treeIndexForPoint(depth+1, samplePos);
			// Check that the index is within the tree
			assert(sampleNodeIndex < numTotalNodes);
			// Check that the index is a leaf node.
			assert(sampleNodeIndex >= m_firstLeafNode);
			TqInt leafIdx = sampleNodeIndex - m_firstLeafNode;
			TqInt leafSubIdx = 0;
			if(!leafOccupied[leafIdx])
			{
				m_depthTree[sampleNodeIndex] = FLT_MAX;
				leafOccupied[leafIdx] = true;
			}
			else
			{
				// Add the initial entry to the leaf list which would have been
				// taken care of in m_depthTree if there had only been one
				// sample for this leaf.
				if(m_leafDepthLists[leafIdx].empty())
					m_leafDepthLists[leafIdx].push_back(FLT_MAX);
				leafSubIdx = m_leafDepthLists[leafIdx].size();
				// The leaf sub-index needs to fit into the number of bits
				// avaliable.
				assert(leafSubIdx < (1 << m_subIndexBits));

				m_leafDepthLists[leafIdx].push_back(FLT_MAX);
			}
			// Pack the leaf index and the index into the array of depths for
			// the leaf together into a single int.
			//
			// NOTE: The number of samples per leaf node is only guarenteed
			// to be small if the sample points are well-stratified.  With
			// m_subIndexBits = 8 we have up to 16*16 samples per leaf node,
			// which should be more than enough to make this code reasonably
			// robust.  That leaves us 24 bits to store the leaf index which
			// lets us deal with buckets of up to 256*256 with 16*16 subsamples
			// which should be sufficient.
			pixel.SampleData(i).occlusionIndex = (leafIdx << m_subIndexBits)
				| (leafSubIdx & ((1 << m_subIndexBits) - 1));
			assert((leafIdx << m_subIndexBits) >> m_subIndexBits == leafIdx);
		}
	}
	// Fix up parent depths.
	propagateDepths();
}


void CqOcclusionTree::setSampleDepth(TqFloat depth, TqInt index)
{
	// Unpack the coded leaf index.
	TqInt leafIdx = index >> m_subIndexBits;
	std::vector<TqFloat>& depths = m_leafDepthLists[leafIdx];
	if(depths.empty())
	{
		// Special case for a single sample per leaf.
		//
		// To be a useful update of the tree, we assume the new sample depth
		// should occlude the previous one.
		assert(m_depthTree[m_firstLeafNode + leafIdx] >= depth);
		m_depthTree[m_firstLeafNode + leafIdx] = depth;
		m_needsUpdate = true;
	}
	else
	{
		// Unpack the index into the array of sample depths for the leaf.
		TqInt leafSubIdx = index & ((1 << m_subIndexBits) - 1);
		assert(depths[leafSubIdx] >= depth);
		depths[leafSubIdx] = depth;
		// Compute the maximum depth of samples within the leaf node of the
		// tree.  Taking the minimum depth wouldn't work, as it would result in
		// the some surfaces being incorrectly culled.  This is what forces us
		// to keep the entire list of depths for each leaf node rather than
		// just a single minimum depth.
		TqFloat max = 0;
		for(std::vector<TqFloat>::iterator d = depths.begin(), end = depths.end();
				d != end; ++d)
		{
			if(max < *d)
				max = *d;
		}
		if(m_depthTree[m_firstLeafNode + leafIdx] > max)
		{
			m_depthTree[m_firstLeafNode + leafIdx] = max;
			m_needsUpdate = true;
		}
	}
}

void CqOcclusionTree::updateTree()
{
	// Only update the depths if the leaf nodes have changed since the last
	// update.
	if(m_needsUpdate)
		propagateDepths();
	m_needsUpdate = false;
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
	TqFloat tminX = max(bound.vecMin().x(), m_treeBoundMin.x());
	TqFloat tminY = max(bound.vecMin().y(), m_treeBoundMin.y());
	TqFloat tmaxX = min(bound.vecMax().x(), m_treeBoundMax.x());
	TqFloat tmaxY = min(bound.vecMax().y(), m_treeBoundMax.y());

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
	assert(p.x() >= 0 && p.x() <= 1);
	assert(p.y() >= 0 && p.y() <= 1);

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

} // namespace Aqsis

