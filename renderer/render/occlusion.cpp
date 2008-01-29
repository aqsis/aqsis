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

START_NAMESPACE( Aqsis )

TqInt CqOcclusionTree::m_Tab = 0;

CqOcclusionTree::CqOcclusionTree(TqInt dimension)
		: m_Parent(0), m_Dimension(dimension)
{
	TqChildArray::iterator child = m_Children.begin();
	for(; child != m_Children.end(); ++child)
		(*child) = 0;
}

CqOcclusionTree::~CqOcclusionTree()
{
	TqChildArray::iterator child = m_Children.begin();
	for(; child != m_Children.end(); ++child)
	{
		if (*child != NULL)
		{
			delete (*child);
			(*child) = NULL;
		}
	};
}

void
CqOcclusionTree::SplitNode(CqOcclusionTreePtr& a, CqOcclusionTreePtr& b)
{
	SortElements(m_Dimension);

	TqInt samplecount = m_SampleIndices.size();
	TqInt median = samplecount / 2;

	// Create the children nodes.
	a = CqOcclusionTreePtr(new CqOcclusionTree());
	b = CqOcclusionTreePtr(new CqOcclusionTree());

	a->m_MinSamplePoint = m_MinSamplePoint;
	b->m_MinSamplePoint = m_MinSamplePoint;
	a->m_MaxSamplePoint = m_MaxSamplePoint;
	b->m_MaxSamplePoint = m_MaxSamplePoint;
	TqInt newdim = ( m_Dimension + 1 ) % Dimensions();
	a->m_Dimension = b->m_Dimension = newdim;

	TqFloat dividingplane = CqBucket::ImageElement(m_SampleIndices[median].first).SampleData(m_SampleIndices[median].second).m_Position[m_Dimension];

	a->m_MaxSamplePoint[m_Dimension] = dividingplane;
	b->m_MinSamplePoint[m_Dimension] = dividingplane;

	TqFloat minTime = m_MaxTime, maxTime = m_MinTime;
	TqInt minDofIndex = m_MaxDofBoundIndex, maxDofIndex = m_MinDofBoundIndex;
	TqFloat minDetailLevel = m_MaxDetailLevel, maxDetailLevel = m_MinDetailLevel;

	TqInt i;
	for(i = 0; i<median; ++i)
	{
		a->m_SampleIndices.push_back(m_SampleIndices[i]);
		const SqSampleData& sample = CqBucket::ImageElement(m_SampleIndices[i].first).SampleData(m_SampleIndices[i].second);
		minTime = MIN(minTime, sample.m_Time);
		maxTime = MAX(maxTime, sample.m_Time);
		minDofIndex = MIN(minDofIndex, sample.m_DofOffsetIndex);
		maxDofIndex = MAX(maxDofIndex, sample.m_DofOffsetIndex);
		minDetailLevel = MIN(minDetailLevel, sample.m_DetailLevel);
		maxDetailLevel = MAX(maxDetailLevel, sample.m_DetailLevel);
	}
	a->m_MinTime = minTime;
	a->m_MaxTime = maxTime;
	a->m_MinDofBoundIndex = minDofIndex;
	a->m_MaxDofBoundIndex = maxDofIndex;
	a->m_MinDetailLevel = minDetailLevel;
	a->m_MaxDetailLevel = maxDetailLevel;

	minTime = m_MaxTime, maxTime = m_MinTime;
	minDofIndex = m_MaxDofBoundIndex, maxDofIndex = m_MinDofBoundIndex;
	minDetailLevel = m_MaxDetailLevel, maxDetailLevel = m_MinDetailLevel;
	for(; i<samplecount; ++i)
	{
		b->m_SampleIndices.push_back(m_SampleIndices[i]);
		const SqSampleData& sample = CqBucket::ImageElement(m_SampleIndices[i].first).SampleData(m_SampleIndices[i].second);
		minTime = MIN(minTime, sample.m_Time);
		maxTime = MAX(maxTime, sample.m_Time);
		minDofIndex = MIN(minDofIndex, sample.m_DofOffsetIndex);
		maxDofIndex = MAX(maxDofIndex, sample.m_DofOffsetIndex);
		minDetailLevel = MIN(minDetailLevel, sample.m_DetailLevel);
		maxDetailLevel = MAX(maxDetailLevel, sample.m_DetailLevel);
	}
	b->m_MinTime = minTime;
	b->m_MaxTime = maxTime;
	b->m_MinDofBoundIndex = minDofIndex;
	b->m_MaxDofBoundIndex = maxDofIndex;
	b->m_MinDetailLevel = minDetailLevel;
	b->m_MaxDetailLevel = maxDetailLevel;
}

void CqOcclusionTree::ConstructTree()
{
	std::deque<CqOcclusionTreePtr> ChildQueue;
	ChildQueue.push_back(this/*shared_from_this()*/);

	TqInt NonLeafCount = NumSamples() >= 1 ? 1 : 0;
	TqInt split_counter = 0;

	while (NonLeafCount > 0 && ChildQueue.size() < s_ChildrenPerNode)
	{
		CqOcclusionTreePtr old = ChildQueue.front();
		ChildQueue.pop_front();
		if (old->NumSamples() > 1)
		{
			--NonLeafCount;
		}

		CqOcclusionTreePtr a;
		CqOcclusionTreePtr b;
		old->SplitNode(a, b);
		split_counter++;
		if (a)
		{
			ChildQueue.push_back(a);
			if (a->NumSamples() > 1)
			{
				++NonLeafCount;
			}
		}
		if (b)
		{
			ChildQueue.push_back(b);
			if (b->NumSamples() > 1)
			{
				++NonLeafCount;
			}
		};

		if(split_counter >1 )
			delete old;
	}

	TqChildArray::iterator ii;
	std::deque<CqOcclusionTreePtr>::const_iterator jj;
	for (ii = m_Children.begin(), jj = ChildQueue.begin(); jj != ChildQueue.end(); ++jj)
	{
		// Check if the child actually has any samples, ignore it if no.
		if( (*jj)->NumSamples() > 0)
		{
			*ii = *jj;
			(*ii)->m_Parent = this/*shared_from_this()*/;
			if ((*ii)->NumSamples() > 1)
			{
				(*ii)->ConstructTree();
			}
			++ii;
		}
	}

	while (ii != m_Children.end())
	{
		if (*ii != NULL)
		{
			delete *ii;
			*ii = NULL;
		}
		ii++;
	}
}


void CqOcclusionTree::InitialiseBounds()
{
	if (m_SampleIndices.size() < 1)
		return;

	const SqSampleData& sample = CqBucket::ImageElement(m_SampleIndices[0].first).SampleData(m_SampleIndices[0].second);
	TqFloat minXVal = sample.m_Position.x();
	TqFloat maxXVal = minXVal;
	TqFloat minYVal = sample.m_Position.y();
	TqFloat maxYVal = minYVal;
	TqFloat minTime = sample.m_Time;
	TqFloat maxTime = minTime;
	TqInt	minDofIndex = sample.m_DofOffsetIndex;
	TqInt	maxDofIndex = minDofIndex;
	TqFloat	minDetailLevel = sample.m_DetailLevel;
	TqFloat	maxDetailLevel = minDetailLevel;
	std::vector<std::pair<TqInt, TqInt> >::iterator i;
	for(i = m_SampleIndices.begin()+1; i!=m_SampleIndices.end(); ++i)
	{
		const SqSampleData& sample = CqBucket::ImageElement(i->first).SampleData(i->second);
		minXVal = MIN(minXVal, sample.m_Position.x());
		maxXVal = MAX(maxXVal, sample.m_Position.x());
		minYVal = MIN(minYVal, sample.m_Position.y());
		maxYVal = MAX(maxYVal, sample.m_Position.y());
		minTime = MIN(minTime, sample.m_Time);
		maxTime = MAX(maxTime, sample.m_Time);
		minDofIndex = MIN(minDofIndex, sample.m_DofOffsetIndex);
		maxDofIndex = MAX(maxDofIndex, sample.m_DofOffsetIndex);
		minDetailLevel = MIN(minDetailLevel, sample.m_DetailLevel);
		maxDetailLevel = MAX(maxDetailLevel, sample.m_DetailLevel);
	}
	m_MinSamplePoint[0] = minXVal;
	m_MaxSamplePoint[0] = maxXVal;
	m_MinSamplePoint[1] = minYVal;
	m_MaxSamplePoint[1] = maxYVal;
	m_MinTime = minTime;
	m_MaxTime = maxTime;
	m_MinDofBoundIndex = minDofIndex;
	m_MaxDofBoundIndex = maxDofIndex;
	m_MinDetailLevel = minDetailLevel;
	m_MaxDetailLevel = maxDetailLevel;
}


void CqOcclusionTree::UpdateBounds()
{
	if (m_Children[0])
	{
		assert(m_SampleIndices.size() > 1);

		TqChildArray::iterator child = m_Children.begin();
		(*child)->UpdateBounds();

		m_MinSamplePoint[0] = (*child)->m_MinSamplePoint[0];
		m_MaxSamplePoint[0] = (*child)->m_MaxSamplePoint[0];
		m_MinSamplePoint[1] = (*child)->m_MinSamplePoint[1];
		m_MaxSamplePoint[1] = (*child)->m_MaxSamplePoint[1];
		m_MinTime = (*child)->m_MinTime;
		m_MaxTime = (*child)->m_MaxTime;
		m_MinDofBoundIndex = (*child)->m_MinDofBoundIndex;
		m_MaxDofBoundIndex = (*child)->m_MaxDofBoundIndex;
		m_MinDetailLevel = (*child)->m_MinDetailLevel;
		m_MaxDetailLevel = (*child)->m_MaxDetailLevel;

		for(++child; child != m_Children.end(); ++child)
		{
			if (*child)
			{
				(*child)->UpdateBounds();

				m_MinSamplePoint[0] = std::min(m_MinSamplePoint[0], (*child)->m_MinSamplePoint[0]);
				m_MaxSamplePoint[0] = std::max(m_MaxSamplePoint[0], (*child)->m_MaxSamplePoint[0]);
				m_MinSamplePoint[1] = std::min(m_MinSamplePoint[1], (*child)->m_MinSamplePoint[1]);
				m_MaxSamplePoint[1] = std::max(m_MaxSamplePoint[1], (*child)->m_MaxSamplePoint[1]);
				m_MinTime = std::min(m_MinTime, (*child)->m_MinTime);
				m_MaxTime = std::max(m_MaxTime, (*child)->m_MaxTime);
				m_MinDofBoundIndex = std::min(m_MinDofBoundIndex, (*child)->m_MinDofBoundIndex);
				m_MaxDofBoundIndex = std::max(m_MaxDofBoundIndex, (*child)->m_MaxDofBoundIndex);
				m_MinDetailLevel = std::min(m_MinDetailLevel, (*child)->m_MinDetailLevel);
				m_MaxDetailLevel = std::max(m_MaxDetailLevel, (*child)->m_MaxDetailLevel);
			}
		}
	}
	else
	{
		assert(m_SampleIndices.size() == 1);

		const SqSampleData& sample = Sample();
		m_MinSamplePoint[0] = m_MaxSamplePoint[0] = sample.m_Position[0];
		m_MinSamplePoint[1] = m_MaxSamplePoint[1] = sample.m_Position[1];
		m_MinTime = m_MaxTime = sample.m_Time;
		m_MinDofBoundIndex = m_MaxDofBoundIndex = sample.m_DofOffsetIndex;
		m_MinDetailLevel = m_MaxDetailLevel = sample.m_DetailLevel;
	}
}

//----------------------------------------------------------------------
// Static Variables

CqBucket* CqOcclusionBox::m_Bucket = NULL;

bool CqOcclusionTree::CqOcclusionTreeComparator::operator()(const std::pair<TqInt, TqInt>& a, const std::pair<TqInt, TqInt>& b)
{
	const SqSampleData& A = CqBucket::ImageElement(a.first).SampleData(a.second);
	const SqSampleData& B = CqBucket::ImageElement(b.first).SampleData(b.second);
	return( A.m_Position[m_Dim] < B.m_Position[m_Dim] );
}


CqOcclusionTreePtr	CqOcclusionBox::m_KDTree;	///< KD Tree representing the samples in the bucket.

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



//----------------------------------------------------------------------
/** Delete the static hierarchy created in CreateHierachy(). static.
*/
void CqOcclusionBox::DeleteHierarchy()
{
	delete m_KDTree;
	m_KDTree = NULL;
}


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

	if(!m_KDTree)
	{
		m_KDTree = CqOcclusionTreePtr(new CqOcclusionTree());
		// Setup the KDTree of samples
		TqInt numpixels = bucket->RealHeight() * bucket->RealWidth();
		TqInt numsamples = bucket->PixelXSamples() * bucket->PixelYSamples();
		for ( TqInt j = 0; j < numpixels; j++ )
		{
			// Gather all samples within the pixel
			for ( TqInt i = 0; i < numsamples; i++ )
			{
				m_KDTree->AddSample(std::pair<TqInt, TqInt>(j,i));
			}
		}
		// Now split the tree down until each leaf has only one sample.
		m_KDTree->InitialiseBounds();
		m_KDTree->ConstructTree();
	}

	m_KDTree->UpdateBounds();

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

			if(node.index * 2 + 1 >= m_depthTree.size())
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


void StoreExtraData( CqMicroPolygon* pMPG, SqImageSample& sample)
{
	std::map<std::string, CqRenderer::SqOutputDataEntry>& DataMap = QGetRenderContext() ->GetMapOfOutputDataEntries();
	std::map<std::string, CqRenderer::SqOutputDataEntry>::iterator entry;
	for ( entry = DataMap.begin(); entry != DataMap.end(); ++entry )
	{
		IqShaderData* pData;
		if ( ( pData = pMPG->pGrid() ->FindStandardVar( entry->first.c_str() ) ) != NULL )
		{
			switch ( pData->Type() )
			{
					case type_float:
					case type_integer:
					{
						TqFloat f;
						pData->GetFloat( f, pMPG->GetIndex() );
						sample.Data()[ entry->second.m_Offset ] = f;
						break;
					}
					case type_point:
					case type_normal:
					case type_vector:
					case type_hpoint:
					{
						CqVector3D v;
						pData->GetPoint( v, pMPG->GetIndex() );
						sample.Data()[ entry->second.m_Offset ] = v.x();
						sample.Data()[ entry->second.m_Offset + 1 ] = v.y();
						sample.Data()[ entry->second.m_Offset + 2 ] = v.z();
						break;
					}
					case type_color:
					{
						CqColor c;
						pData->GetColor( c, pMPG->GetIndex() );
						sample.Data()[ entry->second.m_Offset ] = c.fRed();
						sample.Data()[ entry->second.m_Offset + 1 ] = c.fGreen();
						sample.Data()[ entry->second.m_Offset + 2 ] = c.fBlue();
						break;
					}
					case type_matrix:
					{
						CqMatrix m;
						pData->GetMatrix( m, pMPG->GetIndex() );
						TqFloat* pElements = m.pElements();
						sample.Data()[ entry->second.m_Offset ] = pElements[ 0 ];
						sample.Data()[ entry->second.m_Offset + 1 ] = pElements[ 1 ];
						sample.Data()[ entry->second.m_Offset + 2 ] = pElements[ 2 ];
						sample.Data()[ entry->second.m_Offset + 3 ] = pElements[ 3 ];
						sample.Data()[ entry->second.m_Offset + 4 ] = pElements[ 4 ];
						sample.Data()[ entry->second.m_Offset + 5 ] = pElements[ 5 ];
						sample.Data()[ entry->second.m_Offset + 6 ] = pElements[ 6 ];
						sample.Data()[ entry->second.m_Offset + 7 ] = pElements[ 7 ];
						sample.Data()[ entry->second.m_Offset + 8 ] = pElements[ 8 ];
						sample.Data()[ entry->second.m_Offset + 9 ] = pElements[ 9 ];
						sample.Data()[ entry->second.m_Offset + 10 ] = pElements[ 10 ];
						sample.Data()[ entry->second.m_Offset + 11 ] = pElements[ 11 ];
						sample.Data()[ entry->second.m_Offset + 12 ] = pElements[ 12 ];
						sample.Data()[ entry->second.m_Offset + 13 ] = pElements[ 13 ];
						sample.Data()[ entry->second.m_Offset + 14 ] = pElements[ 14 ];
						sample.Data()[ entry->second.m_Offset + 15 ] = pElements[ 15 ];
						break;
					}
					default:
					// left blank to avoid compiler warnings about unhandled
					//  types
					break;
			}
		}
	}
}



void CqOcclusionTree::SampleMPG( CqMicroPolygon* pMPG, const CqBound& bound, bool usingMB, TqFloat time0, TqFloat time1, bool usingDof, TqInt dofboundindex, SqMpgSampleInfo& MpgSampleInfo, bool usingLOD, SqGridInfo& gridInfo)
{
	// Check the current tree level, and if only one leaf, sample the MP, otherwise, pass it down to the left
	// and/or right side of the tree if it crosses.
	if(NumSamples() == 1)
	{
		// Sample the MPG
		SqSampleData& sample = Sample();
		bool SampleHit;
		TqFloat D;

		CqStats::IncI( CqStats::SPL_count );
		SampleHit = pMPG->Sample(sample, D, sample.m_Time, usingDof );

		if ( SampleHit )
		{
			bool Occludes = MpgSampleInfo.m_Occludes;
			bool opaque =  MpgSampleInfo.m_IsOpaque;

			SqImageSample& currentOpaqueSample = sample.m_OpaqueSample;
			static SqImageSample localImageVal;

			SqImageSample& ImageVal = opaque ? currentOpaqueSample : localImageVal;

			std::deque<SqImageSample>& aValues = sample.m_Data;

			// return if the sample is occluded and can be culled.
			// TODO: should this only be done if the current sample is opaque? Why?
			if(opaque)
			{
				if((currentOpaqueSample.m_flags & SqImageSample::Flag_Valid) &&
					currentOpaqueSample.Data()[Sample_Depth] <= D)
				{
					return;
				}
			}

			CqStats::IncI( CqStats::SPL_hits );
			pMPG->MarkHit();

			TqFloat* val = ImageVal.Data();
			const CqColor& col = MpgSampleInfo.m_Colour;
			const CqColor& opa = MpgSampleInfo.m_Opacity;
			val[ 0 ] = col[0];
			val[ 1 ] = col[1];
			val[ 2 ] = col[2];
			val[ 3 ] = opa[0];
			val[ 4 ] = opa[1];
			val[ 5 ] = opa[2];
			val[ 6 ] = D;

			// Now store any other data types that have been registered.
			if(gridInfo.m_UsesDataMap)
			{
				StoreExtraData(pMPG, ImageVal);
			}

			// \note: There used to be a test here to see if the current sample is 'exactly'
			// the same depth as the nearest in the list, ostensibly to check for a 'grid line' hit
			// but it didn't make sense, so was removed.

			if(pMPG->pGrid()->usesCSG())
				ImageVal.m_pCSGNode = pMPG->pGrid() ->pCSGNode();

			ImageVal.m_flags = 0;
			if ( Occludes )
			{
				ImageVal.m_flags |= SqImageSample::Flag_Occludes;
			}
			if( gridInfo.m_IsMatte )
			{
				ImageVal.m_flags |= SqImageSample::Flag_Matte;
			}

			if(!opaque)
			{
				aValues.push_back( ImageVal );
			}
			else
			{
				// mark this sample as having been written into.
				ImageVal.m_flags |= SqImageSample::Flag_Valid;
			}
		}
	}
	else
	{
		TqChildArray::iterator child;
		TqChildArray::iterator end = m_Children.end();
		for(child = m_Children.begin(); child != end; ++child)
		{
			if (!*child)
				continue;

			if(	   (!usingDof || ((dofboundindex >= (*child)->m_MinDofBoundIndex) && (dofboundindex <= (*child)->m_MaxDofBoundIndex )) )
			        && (!usingMB || ((time0 <= (*child)->m_MaxTime) && (time1 >= (*child)->m_MinTime)) )
			        && (!usingLOD || ((gridInfo.m_LodBounds[0] <= (*child)->m_MaxDetailLevel) && (gridInfo.m_LodBounds[1] >= (*child)->m_MinDetailLevel)) )
			        && (bound.Intersects((*child)->m_MinSamplePoint, (*child)->m_MaxSamplePoint)) )
			{
				//if(bound.vecMin().z() <= (*child)->m_MaxOpaqueZ || !gridInfo.m_IsCullable)
				{
					(*child)->SampleMPG(pMPG, bound, usingMB, time0, time1, usingDof, dofboundindex, MpgSampleInfo, usingLOD, gridInfo);
				}
			}
		}
	}
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


/*void CqOcclusionTree::OutputTree(const char* name)
{
	std::ofstream strFile(name, std::ios_base::out|std::ios_base::app);
 
	strFile <<
			"(" << m_Tab << ", " <<  
			"(" << m_MinSamplePoint[0] << ", " << m_MinSamplePoint[1] << "), " << 
			"(" << m_MaxSamplePoint[0] << ", " << m_MinSamplePoint[1] << "), " <<
			"(" << m_MaxSamplePoint[0] << ", " << m_MinSamplePoint[1] << "), " <<
			"(" << m_MaxSamplePoint[0] << ", " << m_MaxSamplePoint[1] << "), " <<
			"(" << m_MaxSamplePoint[0] << ", " << m_MaxSamplePoint[1] << "), " <<
			"(" << m_MinSamplePoint[0] << ", " << m_MaxSamplePoint[1] << "), " <<
			"(" << m_MinSamplePoint[0] << ", " << m_MaxSamplePoint[1] << "), " <<
			"(" << m_MinSamplePoint[0] << ", " << m_MinSamplePoint[1] << ")" <<
			"), " << 
			std::endl;
 
	TqChildArray::iterator child;
	for(child = m_Children.begin(); child != m_Children.end(); ++child)
	{
		if (*child && (*child)->NumSamples() > 1)
		{
			m_Tab++;
			(*child)->OutputTree(name);
			m_Tab--;
		}
	}
}
*/

END_NAMESPACE( Aqsis )

