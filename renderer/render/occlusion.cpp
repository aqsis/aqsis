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
		\brief Implements the hierarchical occlusion culling class.
		\author Andy Gill (billybobjimboy@users.sf.net)
*/

#include "aqsis.h"

#if _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "occlusion.h"
#include "bound.h"
#include "imagebuffer.h"
#include <deque>
#include <fstream>

namespace Aqsis {

//----------------------------------------------------------------------
// Static Variables

CqBucket* CqOcclusionBox::m_Bucket = NULL;
std::vector<CqOcclusionBox::SqOcclusionNode>	CqOcclusionBox::m_depthTree;
TqLong	CqOcclusionBox::m_firstTerminalNode;


//----------------------------------------------------------------------
/** Constructor
*/

CqOcclusionBox::CqOcclusionBox()
{}


//----------------------------------------------------------------------
/** Destructor
*/

CqOcclusionBox::~CqOcclusionBox()
{}

//------------------------------------------------------------------------------
/** Return the tree index for leaf node containing the given point.
 *
 * treeDepth - depth of the tree, (root node has treeDepth == 1).
 *
 * p - a point which we'd like the index for.  We assume that the possible
 *     points lie in the box [0,1) x [0,1)
 *
 * Returns an index for a 0-based array.
 */
TqUint treeIndexForPoint_fast(TqUint treeDepth, const CqVector2D& p)
{
	assert(treeDepth > 0);
	assert(p.x() >= 0 && p.x() <= 1);
	assert(p.y() >= 0 && p.y() <= 1);

	const TqUint numXSubdivisions = treeDepth / 2;
	const TqUint numYSubdivisions = (treeDepth-1) / 2;
	// true if the last subdivison was along the x-direction.
	const bool lastSubdivisionInX = (treeDepth % 2 == 0);

	// integer coordinates of the point in terms of the subdivison which it
	// falls into, staring from 0 in the top left.
	TqUint x = static_cast<int>(floor(p.x() * (1 << numXSubdivisions)));
	TqUint y = static_cast<int>(floor(p.y() * (1 << numYSubdivisions)));

	// This is the base coordinate for the first leaf in a tree of depth "treeDepth".
	TqUint index = 1 << (treeDepth-1);
	if(lastSubdivisionInX)
	{
		// Every second bit of the index (starting with the LSB) should make up
		// the coordinate x, so distribute x into index in that fashion.
		//
		// Similarly for y, except alternating with the bits of x (starting
		// from the bit up from the LSB.)
		for(TqUint i = 0; i < numXSubdivisions; ++i)
		{
			index |= (x & (1 << i)) << i
				| (y & (1 << i)) << (i+1);
		}
	}
	else
	{
		// This is the opposite of the above: x and y are interlaced as before,
		// but now the LSB of y rather than x goes into the LSB of index.
		for(TqUint i = 0; i < numYSubdivisions; ++i)
		{
			index |= (y & (1 << i)) << i
				| (x & (1 << i)) << (i+1);
		}
	}
	return index - 1;
}

//----------------------------------------------------------------------
/** Setup the hierarchy for one bucket. Static.
	This should be called before rendering each bucket
 *\param bucket: the bucket we are about to render
 *\param xMin: left edge of this bucket (taking into account crop windows etc)
 *\param yMin: Top edge of this bucket
 *\param xMax: Right edge of this bucket
 *\param yMax: Bottom edge of this bucket
*/

void CqOcclusionBox::SetupHierarchy( CqBucket* bucket, TqInt xMin, TqInt yMin, TqInt xMax, TqInt yMax )
{
	assert( bucket );
	m_Bucket = bucket;

	// Now setup the new array based tree.
	// First work out how deep the tree needs to be.
	TqLong numLeafNodes = (bucket->RealHeight() * bucket->RealWidth()) * (bucket->PixelXSamples() * bucket->PixelYSamples());
	TqLong depth = lceil(log10(static_cast<double>(numLeafNodes))/log10(2.0));
	TqLong numTotalNodes = lceil(pow(2.0, static_cast<double>(depth+1)))-1;
	m_firstTerminalNode = lceil(pow(2.0, static_cast<double>(depth)))-1;
	m_depthTree.clear();
	m_depthTree.resize(numTotalNodes, SqOcclusionNode(0.0f));

	// Now initialise the node depths of those that contain sample points to infinity.
	std::vector<SqSampleData>& samples = bucket->SamplePoints();
	std::vector<SqSampleData>::iterator sample;
	for(sample = samples.begin(); sample != samples.end(); ++sample)
	{
		CqVector2D samplePos = sample->m_Position;
		samplePos.x((samplePos.x() - static_cast<TqFloat>(bucket->realXOrigin())) / static_cast<TqFloat>(bucket->RealWidth()));
		samplePos.y((samplePos.y() - static_cast<TqFloat>(bucket->realYOrigin())) / static_cast<TqFloat>(bucket->RealHeight()));
		TqUint sampleNodeIndex = treeIndexForPoint_fast(depth+1, samplePos);

		// Check that the index is within the tree
		assert(sampleNodeIndex < numTotalNodes);
		// Check that the index is a leaf node.
		assert((sampleNodeIndex*2)+1 >= numTotalNodes);

		m_depthTree[sampleNodeIndex].depth = FLT_MAX;
		TqInt parentIndex = (sampleNodeIndex-1)>>1;
		while(parentIndex >= 0)
		{
			m_depthTree[parentIndex].depth = FLT_MAX;
			parentIndex = (parentIndex - 1)>>1;
		}
		m_depthTree[sampleNodeIndex].samples.push_back(&(*sample));
	} 
}

struct SqNodeStack
{
	SqNodeStack(TqFloat _minX, TqFloat _minY, TqFloat _maxX, TqFloat _maxY, TqInt _index, bool _splitInX) :
		minX(_minX), minY(_minY), maxX(_maxX), maxY(_maxY), index(_index), splitInX(_splitInX) {}
	TqFloat minX, minY;
	TqFloat maxX, maxY;
	TqInt	index;
	bool	splitInX;
};

bool CqOcclusionBox::CanCull( CqBound* bound )
{
	// Normalize the bound
	TqFloat minX = m_Bucket->realXOrigin();
	TqFloat minY = m_Bucket->realYOrigin(); 
	TqFloat maxX = m_Bucket->realXOrigin() + m_Bucket->RealWidth();
	TqFloat maxY = m_Bucket->realYOrigin() + m_Bucket->RealHeight();
	// If the test bound is bigger than the overall bucket, then crop it.
	TqFloat tminX = std::max(bound->vecMin().x(), minX);
	TqFloat tminY = std::max(bound->vecMin().y(), minY);
	TqFloat tmaxX = std::min(bound->vecMax().x(), maxX);
	TqFloat tmaxY = std::min(bound->vecMax().y(), maxY);

	std::deque<SqNodeStack> stack;
	stack.push_front(SqNodeStack(minX, minY, maxX, maxY, 0, true));
	bool cull = true;
	SqNodeStack terminatingNode(0.0f, 0.0f, 0.0f, 0.0f, 0, true);
	
	while(!stack.empty())
	{
		SqNodeStack node = stack.front();
		stack.pop_front();

		// If the bound intersects the node.
		if( !(tminX > maxX || tminY > maxY ||
			  tmaxX < minX || tmaxY < minY) )
		{
			// If the depth stored at the node is closer than the minimum Z of the bound, contiune.
			if(m_depthTree[node.index].depth < bound->vecMin().z()) 
				continue;

			if(node.index * 2 + 1 >= static_cast<TqInt>(m_depthTree.size()))
			{
				cull = false;
				break;
			}

			if(node.splitInX)
			{
				TqFloat median = ((node.maxX - node.minX) * 0.5f) + node.minX;
				stack.push_front(SqNodeStack(node.minX, node.minY, median, node.maxY, node.index * 2 + 1, !node.splitInX));
				stack.push_front(SqNodeStack(median, node.minY, node.maxX, node.maxY, node.index * 2 + 2, !node.splitInX));
			}
			else
			{
				TqFloat median = ((node.maxY - node.minY) * 0.5f) + node.minY;
				stack.push_front(SqNodeStack(node.minX, node.minY, node.maxX, median, node.index * 2 + 1, !node.splitInX));
				stack.push_front(SqNodeStack(node.minX, median, node.maxX, node.maxY, node.index * 2 + 2, !node.splitInX));
			}
		}
	}
	return(cull);
}


TqFloat CqOcclusionBox::propagateDepths(std::vector<CqOcclusionBox::SqOcclusionNode>::size_type index)
{
	// Check if it's a terminal node.
	if((index * 2)+1 < m_depthTree.size())
	{
		// Get the depths of the children
		TqFloat d1 = propagateDepths((index*2)+1);
		TqFloat d2 = propagateDepths((index*2)+2);
		m_depthTree[index].depth = std::max(d1, d2);
	}
	return(m_depthTree[index].depth);
}


void CqOcclusionBox::RefreshDepthMap()
{
	// First zero the terminal nodes.
	// Could probably do this more efficiently by only zeroing the terminal ones.
	//std::fill(m_depthTree.begin(), m_depthTree.end(), SqOcclusionNode(FLT_MAX));
	if(m_depthTree.size() == 0)
		return;

	// Now set the terminal node depths to the furthest of the sample points they contain.
	std::vector<SqOcclusionNode>::iterator node = m_depthTree.begin();
	node += m_firstTerminalNode;
	for(; node != m_depthTree.end(); ++node)
	{
		if(node->samples.size() > 0)
		{
			std::vector<SqSampleData*>::iterator sample;
			TqFloat max = 0;
			bool hit = false;
			for(sample = node->samples.begin(); sample != node->samples.end(); ++sample)
			{
				if((*sample)->m_OpaqueSample.m_flags & SqImageSample::Flag_Valid &&
				   (*sample)->m_OpaqueSample.Data()[Sample_Depth] > max)
				{
					max = (*sample)->m_OpaqueSample.Data()[Sample_Depth];
					hit = true;
				}
			}
			node->depth = (hit)?max:FLT_MAX;
		}
	}
	
	// Now propagate the changes up the tree.
	propagateDepths(0);
}


} // namespace Aqsis

