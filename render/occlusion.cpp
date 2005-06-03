// Aqsis
// Copyright © 1997 - 2002, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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

#include "occlusion.h"
#include "bound.h"
#include "imagebuffer.h"

START_NAMESPACE( Aqsis )


void CqOcclusionTree::ConstructTree()
{
	SortElements(m_Dimension);

	// Create the children nodes.
	CqOcclusionTree* a = new CqOcclusionTree();
	CqOcclusionTree* b = new CqOcclusionTree();
	TqInt samplecount = Samples().size();
	TqInt median = static_cast<TqInt>( m_Samples.size() / 2.0f );
	TqFloat dividingplane = m_Samples[median]->m_Position[m_Dimension];

	a->m_MinSamplePoint = m_MinSamplePoint;
	b->m_MinSamplePoint = m_MinSamplePoint;
	a->m_MaxSamplePoint = m_MaxSamplePoint;
	b->m_MaxSamplePoint = m_MaxSamplePoint;
    TqInt newdim = ( m_Dimension + 1 ) % Dimensions();
	a->m_Dimension = b->m_Dimension = newdim;

	a->m_MaxSamplePoint[m_Dimension] = dividingplane;
	b->m_MinSamplePoint[m_Dimension] = dividingplane;

	TqFloat minTime = m_MaxTime, maxTime = m_MinTime;
	TqFloat minDofIndex = m_MaxDofBoundIndex, maxDofIndex = m_MinDofBoundIndex;

	TqInt i;
	for(i = 0; i<median; ++i)
	{
		a->Samples().push_back(Samples()[i]);
		minTime = MIN(minTime, Samples()[0]->m_Time);
		maxTime = MAX(maxTime, Samples()[0]->m_Time);
		minDofIndex = MIN(minDofIndex, Samples()[0]->m_DofOffsetIndex);
		maxDofIndex = MAX(maxDofIndex, Samples()[0]->m_DofOffsetIndex);
	}
	a->m_MinTime = minTime;
	a->m_MaxTime = maxTime;
	a->m_MinDofBoundIndex = minDofIndex;
	a->m_MaxDofBoundIndex = maxDofIndex;

	minTime = m_MaxTime, maxTime = m_MinTime;
	minDofIndex = m_MaxDofBoundIndex, maxDofIndex = m_MinDofBoundIndex;
	for(; i<samplecount; ++i)
	{
		b->Samples().push_back(Samples()[i]);
		minTime = MIN(minTime, Samples()[0]->m_Time);
		maxTime = MAX(maxTime, Samples()[0]->m_Time);
		minDofIndex = MIN(minDofIndex, Samples()[0]->m_DofOffsetIndex);
		maxDofIndex = MAX(maxDofIndex, Samples()[0]->m_DofOffsetIndex);
	}
	b->m_MinTime = minTime;
	b->m_MaxTime = maxTime;
	b->m_MinDofBoundIndex = minDofIndex;
	b->m_MaxDofBoundIndex = maxDofIndex;

	if(a->Samples().size() >= 1)
	{
		m_Children.push_back(a);
		a->m_Parent = this;
		if(a->Samples().size() > 1)
			a->ConstructTree();
	}
	if(b->Samples().size() >= 1)
	{
		m_Children.push_back(b);
		b->m_Parent = this;
		if(b->Samples().size() > 1)
			b->ConstructTree();
	}
}


void CqOcclusionTree::InitialiseBounds()
{
	if (Samples().size() < 1) return;

	TqFloat minXVal = Samples()[0]->m_Position.x();
	TqFloat maxXVal = minXVal;
	TqFloat minYVal = Samples()[0]->m_Position.y();
	TqFloat maxYVal = minYVal;
	TqFloat minTime = Samples()[0]->m_Time;
	TqFloat maxTime = minTime;
	TqInt	minDofIndex = Samples()[0]->m_DofOffsetIndex;
	TqInt	maxDofIndex = minDofIndex;
	std::vector<SqSampleData*>::iterator i;
	for(i = Samples().begin(); i!=Samples().end(); ++i)
	{
		minXVal = MIN(minXVal, (*i)->m_Position.x());
		maxXVal = MAX(maxXVal, (*i)->m_Position.x());
		minYVal = MIN(minYVal, (*i)->m_Position.y());
		maxYVal = MAX(maxYVal, (*i)->m_Position.y());
		minTime = MIN(minTime, (*i)->m_Time);
		maxTime = MAX(maxTime, (*i)->m_Time);
		minDofIndex = MIN(minDofIndex, (*i)->m_DofOffsetIndex);
		maxDofIndex = MAX(maxDofIndex, (*i)->m_DofOffsetIndex);
	}
	m_MinSamplePoint[0] = minXVal;
	m_MaxSamplePoint[0] = maxXVal;
	m_MinSamplePoint[1] = minYVal;
	m_MaxSamplePoint[1] = maxYVal;
	m_MinTime = minTime;
	m_MaxTime = maxTime;
	m_MinDofBoundIndex = minDofIndex;
	m_MaxDofBoundIndex = maxDofIndex;

	// Set the opaque depths to the limits to begin with.
	m_MaxOpaqueZ = FLT_MAX;
}


void CqOcclusionTree::UpdateBounds()
{
	// Update the children first
	if(m_Children.size() >= 1)
	{
		std::vector<CqOcclusionTree*>::iterator child;
		for(child = m_Children.begin(); child != m_Children.end(); ++child)
			(*child)->UpdateBounds();
	}

	if(m_Children.size() == 0)
	{
		assert(Samples().size() == 1);
		SqSampleData* sample = Samples()[0];
		m_MinSamplePoint[0] = m_MaxSamplePoint[0] = sample->m_Position[0];
		m_MinSamplePoint[1] = m_MaxSamplePoint[1] = sample->m_Position[1];
		m_MinTime = m_MaxTime = sample->m_Time;
		m_MinDofBoundIndex = m_MaxDofBoundIndex = sample->m_DofOffsetIndex;
	}
	else
	{
		TqFloat minXVal = m_Children[0]->m_MinSamplePoint[0];
		TqFloat maxXVal = minXVal;
		TqFloat minYVal = m_Children[0]->m_MinSamplePoint[1];
		TqFloat maxYVal = minYVal;
		TqFloat minTime = m_Children[0]->m_MinTime;
		TqFloat maxTime = minTime;
		TqInt	minDofIndex = m_Children[0]->m_MinDofBoundIndex;
		TqInt	maxDofIndex = minDofIndex;
		std::vector<CqOcclusionTree*>::iterator i;
		for(i = m_Children.begin(); i!=m_Children.end(); ++i)
		{
			minXVal = MIN(minXVal, (*i)->m_MinSamplePoint[0]);
			maxXVal = MAX(maxXVal, (*i)->m_MaxSamplePoint[0]);
			minYVal = MIN(minYVal, (*i)->m_MinSamplePoint[1]);
			maxYVal = MAX(maxYVal, (*i)->m_MaxSamplePoint[1]);
			minTime = MIN(minTime, (*i)->m_MinTime);
			maxTime = MAX(maxTime, (*i)->m_MaxTime);
			minDofIndex = MIN(minDofIndex, (*i)->m_MinDofBoundIndex);
			maxDofIndex = MAX(maxDofIndex, (*i)->m_MaxDofBoundIndex);
		}
		m_MinSamplePoint[0] = minXVal;
		m_MaxSamplePoint[0] = maxXVal;
		m_MinSamplePoint[1] = minYVal;
		m_MaxSamplePoint[1] = maxYVal;
		m_MinTime = minTime;
		m_MaxTime = maxTime;
		m_MinDofBoundIndex = minDofIndex;
		m_MaxDofBoundIndex = maxDofIndex;
	}
	// Set the opaque depths to the limits to begin with.
	m_MaxOpaqueZ = FLT_MAX;
}

void CqOcclusionTree::PropagateChanges()
{
	// Update our opaque depth based on that our our children.
	if( m_Children.size() > 0 )
	{
		TqFloat maxdepth = -FLT_MAX;
		std::vector<CqOcclusionTree*>::iterator child;
		for(child = m_Children.begin(); child != m_Children.end(); ++child)
			maxdepth = MAX((*child)->m_MaxOpaqueZ, maxdepth);
		// Only if this has resulted in a change at this level, should we process the parent.
		if(maxdepth < m_MaxOpaqueZ)
		{
			m_MaxOpaqueZ = maxdepth;
			if( m_Parent)
				m_Parent->PropagateChanges();
		}
	}
	else
	{
		if( m_Parent)
			m_Parent->PropagateChanges();
	}
}

//----------------------------------------------------------------------
// Static Variables

CqBucket* CqOcclusionBox::m_Bucket = NULL;

bool CqOcclusionTree::CqOcclusionTreeComparator::operator()(SqSampleData* a, SqSampleData* b)
{
    return( a->m_Position[m_Dim] < b->m_Position[m_Dim] );
}


CqOcclusionTree	CqOcclusionBox::m_KDTree;	///< KD Tree representing the samples in the bucket.


//----------------------------------------------------------------------
/** Constructor
*/

CqOcclusionBox::CqOcclusionBox()
{
}


//----------------------------------------------------------------------
/** Destructor
*/

CqOcclusionBox::~CqOcclusionBox()
{}


//----------------------------------------------------------------------
/** Create the static hierarchy that will be used throughout the rendering. Static.
	This should be called only once, before rendering has started.
 *\param bucketXSize width of a bucket in pixels
 *\param bucketYSize height of a bucket in pixels
 *\param XFWidth  filter width in x
 *\param YFWidth  filter width in y
*/

void CqOcclusionBox::CreateHierarchy( TqInt bucketXSize, TqInt bucketYSize, TqInt XFWidth, TqInt YFWidth )
{
	// Initialise the data handler for the KDTree
	//CqOcclusionKDTreeData*	kddata = new CqOcclusionKDTreeData;
	//m_KDTree.SetData(boost::shared_ptr<IqKDTreeData<TqOcclusionKDTreeData,SqOcclusionKDTreeExtraData> >(kddata));
}


//----------------------------------------------------------------------
/** Delete the static hierarchy created in CreateHierachy(). static.
*/
void CqOcclusionBox::DeleteHierarchy()
{
    m_KDTree.m_Samples.clear();
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

	if(m_KDTree.Samples().empty())
	{
		// Setup the KDTree of samples
		m_KDTree.Samples().clear();
		for ( TqInt j = yMin; j < yMax; j++ )
		{
			for ( TqInt i = xMin; i < xMax; i++ )
			{
				CqImagePixel* pie;
				m_Bucket->ImageElement( i, j, pie );
				// Gather all samples within the pixel
				TqInt sampleIndex = 0;
				TqInt sx, sy;
				for ( sy = 0; sy < pie->YSamples(); sy++ )
				{
					for ( sx = 0; sx < pie->XSamples(); sx++ )
					{
						SqSampleData* pSample = &pie->SampleData( sampleIndex );
						m_KDTree.Samples().push_back(pSample);
						sampleIndex++;
					}
				}
			}
		}
		// Now split the tree down until each leaf has only one sample.
		m_KDTree.InitialiseBounds();
		m_KDTree.ConstructTree();
	}

	m_KDTree.UpdateBounds();
}


END_NAMESPACE( Aqsis )

