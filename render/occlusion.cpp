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

//----------------------------------------------------------------------
// Static Variables

CqBucket* CqOcclusionBox::m_Bucket = NULL;

bool CqOcclusionKDTreeData::CqOcclusionKDTreeDataComparator::operator()(TqOcclusionKDTreeData a, TqOcclusionKDTreeData b)
{
    return( a->m_Position[m_Dim] < b->m_Position[m_Dim] );
}


TqOcclusionKDTree	CqOcclusionBox::m_KDTree;	///< KD Tree representing the samples in the bucket.


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
	CqOcclusionKDTreeData*	kddata = new CqOcclusionKDTreeData;
	m_KDTree.SetData(boost::shared_ptr<IqKDTreeData<TqOcclusionKDTreeData,SqOcclusionKDTreeExtraData> >(kddata));
}


//----------------------------------------------------------------------
/** Delete the static hierarchy created in CreateHierachy(). static.
*/
void CqOcclusionBox::DeleteHierarchy()
{
    m_KDTree.aLeaves().clear();
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

	if(m_KDTree.aLeaves().empty())
	{
		// Setup the KDTree of samples
		m_KDTree.aLeaves().clear();
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
						m_KDTree.aLeaves().push_back(pSample);
						sampleIndex++;
					}
				}
			}
		}
		// Setup the limits of the initial level
		m_KDTree.Initialise(TqFalse);
		// Now split the tree down until each leaf has only one sample.
		m_KDTree.Subdivide(1);
	}

	m_KDTree.Initialise(TqTrue);
}


END_NAMESPACE( Aqsis )

