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

#include "aqsis.h"

#ifdef	AQSIS_SYSTEM_WIN32
#pragma warning(disable : 4786)
#endif

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
	TqInt samplecount = m_SampleIndices.size();
	TqInt median = static_cast<TqInt>( m_SampleIndices.size() / 2.0f );
	TqFloat dividingplane = CqBucket::ImageElement(m_SampleIndices[median].first).SampleData(m_SampleIndices[median].second).m_Position[m_Dimension];

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
		a->m_SampleIndices.push_back(m_SampleIndices[i]);
		const SqSampleData& sample = CqBucket::ImageElement(m_SampleIndices[i].first).SampleData(m_SampleIndices[i].second);
		minTime = MIN(minTime, sample.m_Time);
		maxTime = MAX(maxTime, sample.m_Time);
		minDofIndex = MIN(minDofIndex, sample.m_DofOffsetIndex);
		maxDofIndex = MAX(maxDofIndex, sample.m_DofOffsetIndex);
	}
	a->m_MinTime = minTime;
	a->m_MaxTime = maxTime;
	a->m_MinDofBoundIndex = minDofIndex;
	a->m_MaxDofBoundIndex = maxDofIndex;

	minTime = m_MaxTime, maxTime = m_MinTime;
	minDofIndex = m_MaxDofBoundIndex, maxDofIndex = m_MinDofBoundIndex;
	for(; i<samplecount; ++i)
	{
		b->m_SampleIndices.push_back(m_SampleIndices[i]);
		const SqSampleData& sample = CqBucket::ImageElement(m_SampleIndices[i].first).SampleData(m_SampleIndices[i].second);
		minTime = MIN(minTime, sample.m_Time);
		maxTime = MAX(maxTime, sample.m_Time);
		minDofIndex = MIN(minDofIndex, sample.m_DofOffsetIndex);
		maxDofIndex = MAX(maxDofIndex, sample.m_DofOffsetIndex);
	}
	b->m_MinTime = minTime;
	b->m_MaxTime = maxTime;
	b->m_MinDofBoundIndex = minDofIndex;
	b->m_MaxDofBoundIndex = maxDofIndex;

	TqChildArray::iterator ii = m_Children.begin();
	if(a->m_SampleIndices.size() >= 1)
	{
		*ii++ = a;
		a->m_Parent = this;
		if(a->m_SampleIndices.size() > 1)
			a->ConstructTree();
		else
			a->m_Children[0] = a->m_Children[1] = 0;
	}
	else
		delete(a);
	
	if(b->m_SampleIndices.size() >= 1)
	{
		*ii++ = b;
		b->m_Parent = this;
		if(b->m_SampleIndices.size() > 1)
			b->ConstructTree();
		else
			b->m_Children[0] = b->m_Children[1] = 0;
	}
	else
		delete(b);

	while (ii != m_Children.end())
	{
	    *ii++ = 0;
	}
}

void CqOcclusionTree::DestroyTree()
{
    m_SampleIndices.clear();
	for(TqChildArray::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		if( (*i) )
		{
			(*i)->DestroyTree();
			delete(*i);
			*i = 0;
		}
	}
}


void CqOcclusionTree::InitialiseBounds()
{
	if (m_SampleIndices.size() < 1) return;

	const SqSampleData& sample = CqBucket::ImageElement(m_SampleIndices[0].first).SampleData(m_SampleIndices[0].second);
	TqFloat minXVal = sample.m_Position.x();
	TqFloat maxXVal = minXVal;
	TqFloat minYVal = sample.m_Position.y();
	TqFloat maxYVal = minYVal;
	TqFloat minTime = sample.m_Time;
	TqFloat maxTime = minTime;
	TqInt	minDofIndex = sample.m_DofOffsetIndex;
	TqInt	maxDofIndex = minDofIndex;
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
	if (m_Children[0])
	{
		TqChildArray::iterator child;
		for(child = m_Children.begin(); child != m_Children.end(); ++child)
		{
			if (*child)
			{
				(*child)->UpdateBounds();
			}
		}
	}

	if (!m_Children[0])
	{
		assert(m_SampleIndices.size() == 1);
		const SqSampleData& sample = Sample();
		m_MinSamplePoint[0] = m_MaxSamplePoint[0] = sample.m_Position[0];
		m_MinSamplePoint[1] = m_MaxSamplePoint[1] = sample.m_Position[1];
		m_MinTime = m_MaxTime = sample.m_Time;
		m_MinDofBoundIndex = m_MaxDofBoundIndex = sample.m_DofOffsetIndex;
	}
	else
	{
		m_MinSamplePoint[0] = m_Children[0]->m_MinSamplePoint[0];
		m_MaxSamplePoint[0] = m_Children[0]->m_MaxSamplePoint[0];
		m_MinSamplePoint[1] = m_Children[0]->m_MinSamplePoint[1];
		m_MaxSamplePoint[1] = m_Children[0]->m_MaxSamplePoint[1];
		m_MinTime = m_Children[0]->m_MinTime;
		m_MaxTime = m_Children[0]->m_MaxTime;
		m_MinDofBoundIndex = m_Children[0]->m_MinDofBoundIndex;
		m_MaxDofBoundIndex = m_Children[0]->m_MaxDofBoundIndex;

		TqChildArray::iterator child = m_Children.begin();

		for (++child; child != m_Children.end(); ++child)
		{
			if (*child)
			{
				m_MinSamplePoint[0] = MIN(m_MinSamplePoint[0], (*child)->m_MinSamplePoint[0]);
				m_MaxSamplePoint[0] = MAX(m_MaxSamplePoint[0], (*child)->m_MaxSamplePoint[0]);
				m_MinSamplePoint[1] = MIN(m_MinSamplePoint[1], (*child)->m_MinSamplePoint[1]);
				m_MaxSamplePoint[1] = MAX(m_MaxSamplePoint[1], (*child)->m_MaxSamplePoint[1]);
				m_MinTime = MIN(m_MinTime, (*child)->m_MinTime);
				m_MaxTime = MAX(m_MaxTime, (*child)->m_MaxTime);
				m_MinDofBoundIndex = MIN(m_MinDofBoundIndex, (*child)->m_MinDofBoundIndex);
				m_MaxDofBoundIndex = MAX(m_MaxDofBoundIndex, (*child)->m_MaxDofBoundIndex);
			}
		}
	}
	// Set the opaque depths to the limits to begin with.
	m_MaxOpaqueZ = FLT_MAX;
}

void CqOcclusionTree::PropagateChanges()
{
	CqOcclusionTree* node = this;
	// Update our opaque depth based on that our our children.
	while(node)
	{
		if( node->m_Children[0] )
		{
			TqFloat maxdepth = m_Children[0]->m_MaxOpaqueZ;
			TqChildArray::iterator child;
			for (++child; child != node->m_Children.end(); ++child)
			{
				if (*child)
				{
					maxdepth = MAX((*child)->m_MaxOpaqueZ, maxdepth);
				}
			}
			// Only if this has resulted in a change at this level, should we process the parent.
			if(maxdepth < node->m_MaxOpaqueZ)
			{
				node->m_MaxOpaqueZ = maxdepth;
				node = node->m_Parent;
			}
			else
				break;
		}
		else
			node = node->m_Parent;
	}
}


TqBool CqOcclusionTree::CanCull( CqBound* bound )
{
	// Recursively call each level to see if it can be culled at that point.
	// Stop recursing at a level that doesn't contain the whole bound.
	std::deque<CqOcclusionTree*> stack;
	stack.push_front(this);
	TqBool	top_level = TqTrue;
	while(!stack.empty())
	{
		CqOcclusionTree* node = stack.front();
		stack.pop_front();
		// Check the bound against the 2D limits of this level, if not entirely contained, then we
		// cannot cull at this level, nor at any of the children.
		CqBound b1(node->MinSamplePoint(), node->MaxSamplePoint());
		if(b1.Contains2D(*bound) || top_level)
		{
			top_level = TqFalse;
			if( bound->vecMin().z() > node->MaxOpaqueZ() )
				// If the bound is entirely contained within this node's 2D bound, and is further
				// away than the furthest opaque point, then cull.
				return(TqTrue);
			// If contained, but not behind the furthest point, push the children nodes onto the stack for
			// processing.
			CqOcclusionTree::TqChildArray::iterator childNode;
			for (childNode = node->m_Children.begin(); childNode != node->m_Children.end(); ++childNode)
			{
				if (*childNode)
				{
					stack.push_front(*childNode);
				}
			}
		}
	}
	return(TqFalse);
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
	m_KDTree.DestroyTree();
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

	if(m_KDTree.NumSamples() == 0)
	{
		// Setup the KDTree of samples
		TqInt numpixels = bucket->RealHeight() * bucket->RealWidth();
		TqInt numsamples = bucket->PixelXSamples() * bucket->PixelYSamples();
		for ( TqInt j = 0; j < numpixels; j++ )
		{
			CqImagePixel& pixel = bucket->ImageElement(j);
			// Gather all samples within the pixel
			for ( TqInt i = 0; i < numsamples; i++ )
			{
				m_KDTree.AddSample(std::pair<TqInt, TqInt>(j,i));
			}
		}
		// Now split the tree down until each leaf has only one sample.
		m_KDTree.InitialiseBounds();
		m_KDTree.ConstructTree();
	}

	m_KDTree.UpdateBounds();
}


TqBool CqOcclusionBox::CanCull( CqBound* bound )
{
	return(m_KDTree.CanCull(bound));
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



void CqOcclusionTree::SampleMPG( CqMicroPolygon* pMPG, const CqBound& bound, TqFloat time0, TqFloat time1, TqBool usingDof, TqInt dofboundindex, SqMpgSampleInfo& MpgSampleInfo, SqGridInfo& gridInfo)
{
	// Check the current tree level, and if only one leaf, sample the MP, otherwise, pass it down to the left
	// and/or right side of the tree if it crosses.
	if(NumSamples() == 1)
	{
		// Sample the MPG
		SqSampleData& sample = Sample();
		TqBool SampleHit;
		TqFloat D;

		const TqFloat* LodBounds = gridInfo.m_LodBounds;
		TqBool UsingLevelOfDetail = LodBounds[ 0 ] >= 0.0f;
		// Check to see if the sample is within the sample's level of detail
		// \note Need to move this to the level check further down and 
		// include LOD bounds in the treenode data.
		if( UsingLevelOfDetail )
		{
			TqFloat LevelOfDetail = sample.m_DetailLevel;
			if ( LodBounds[ 0 ] > LevelOfDetail || LevelOfDetail >= LodBounds[ 1 ] )
				return;
		}

		CqStats::IncI( CqStats::SPL_count );
		SampleHit = pMPG->Sample(sample, D, sample.m_Time, usingDof );

		if ( SampleHit )
		{
			TqBool Occludes = MpgSampleInfo.m_Occludes;
			TqBool opaque =  MpgSampleInfo.m_IsOpaque;

			SqImageSample& currentOpaqueSample = sample.m_OpaqueSample;
			static SqImageSample localImageVal;

			SqImageSample& ImageVal = opaque ? currentOpaqueSample : localImageVal;

			std::list<SqImageSample>& aValues = sample.m_Data;
			std::list<SqImageSample>::iterator sample = aValues.begin();
			std::list<SqImageSample>::iterator end = aValues.end();

			// return if the sample is occluded and can be culled.
			if(opaque)
			{
				if((currentOpaqueSample.m_flags & SqImageSample::Flag_Valid) &&
					currentOpaqueSample.Data()[Sample_Depth] <= D)
				{
					return;
				}
			}
			else
			{
				// Sort the color/opacity into the visible point list
				// return if the sample is occluded and can be culled.
				while( sample != end )
				{
					if((*sample).Data()[Sample_Depth] >= D)
						break;

					if(((*sample).m_flags & SqImageSample::Flag_Occludes) &&
						!(*sample).m_pCSGNode && gridInfo.m_IsCullable)
						return;

					++sample;
				}
			}

			ImageVal.Data()[Sample_Depth] = D;

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

			if(!opaque)
			{
				// If depth is exactly the same as previous sample, chances are we've
				// hit a MPG grid line.
				// \note: Cannot do this if there is CSG involved, as all samples must be taken and kept the same.
				if ( sample != end && (*sample).Data()[Sample_Depth] == ImageVal.Data()[Sample_Depth] && !(*sample).m_pCSGNode )
				{
				    TqInt datasize = QGetRenderContext()->GetOutputDataTotalSize();
					TqInt i;
					for( i=0; i<datasize; ++i)
						(*sample).Data()[i] = ( (*sample).Data()[i] + ImageVal.Data()[i] ) * 0.5f;
					return;
				}
			}

			// Update max depth values if the sample is opaque and can occlude
			// If the sample depth is closer than the current closest one, and is opaques
			// we can just replace, as we know we are in a treenode that is a leaf.
			if ( opaque )
			{
				if(D < MaxOpaqueZ())
				{
					SetMaxOpaqueZ(D);
					PropagateChanges();
				}
			}

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
				aValues.insert( sample, ImageVal );
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
		for(child = m_Children.begin(); child != m_Children.end(); ++child)
		{
			if (!*child)
			{
				continue;
			}

			if(	(!usingDof || (dofboundindex >= (*child)->m_MinDofBoundIndex)
								&& (dofboundindex <= (*child)->m_MaxDofBoundIndex ))
				&& ((time0 <= (*child)->m_MaxTime) && (time1 >= (*child)->m_MinTime) )
				&& (bound.Intersects((*child)->m_MinSamplePoint, (*child)->m_MaxSamplePoint)) )
			{
				if(bound.vecMin().z() <= (*child)->m_MaxOpaqueZ || !gridInfo.m_IsCullable)
				{
					(*child)->SampleMPG(pMPG, bound, time0, time1, usingDof, dofboundindex, MpgSampleInfo, gridInfo);
				}
			}
		}
	}
}


END_NAMESPACE( Aqsis )

