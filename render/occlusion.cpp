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

#ifdef AQSIS_SYSTEM_MACOSX 
// Workaround for Mac OS X gcc 2.95 compiler error -
// "Fixup [linenumber] too large for field width of 16 bits"
#pragma CC_OPT_OFF
#endif

//----------------------------------------------------------------------
// Static Variables

CqBucket* CqOcclusionBox::m_Bucket = NULL;
CqOcclusionBox* CqOcclusionBox::m_Hierarchy = NULL;
TqInt CqOcclusionBox::m_HierarchyLevels = 0;
TqInt CqOcclusionBox::m_TotalBoxes = 0;
TqInt* CqOcclusionBox::m_LevelStartId = NULL;




//----------------------------------------------------------------------
/** Constructor
*/

CqOcclusionBox::CqOcclusionBox()
{
	m_MinX = 0;
	m_MinY = 0;
	m_MaxX = 0;
	m_MaxY = 0;

	m_MinZ = FLT_MAX;
	m_MaxZ = FLT_MAX;

	m_Id = -1; // Start off uninitialised

	m_NeedsUpdating = TqFalse;
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
	// Make sure this hasn't been called more than once;
	assert( m_Hierarchy == NULL );
	assert( m_LevelStartId == NULL );

	int xdivisions = ( int ) ( log10( bucketXSize + XFWidth ) / log10( 2.0 ) );
	int ydivisions = ( int ) ( log10( bucketYSize + YFWidth ) / log10( 2.0 ) );

	m_HierarchyLevels = MIN( xdivisions, ydivisions );

	m_LevelStartId = new int[ m_HierarchyLevels + 1 ];
	m_LevelStartId[ 0 ] = 0;
	int numBoxes = 1;
	TqInt i;
	for ( i = 1; i < m_HierarchyLevels; i++ )
	{
		m_LevelStartId[ i ] = numBoxes;
		numBoxes = numBoxes * 4 + 1;
	}
	m_LevelStartId[ m_HierarchyLevels ] = numBoxes;
	m_TotalBoxes = numBoxes;

	m_Hierarchy = new CqOcclusionBox[ numBoxes ];

	for ( i = 0; i < numBoxes; i++ )
	{
		m_Hierarchy[ i ].m_Id = i;
	}
}


//----------------------------------------------------------------------
/** Delete the static hierarchy created in CreateHierachy(). static.
*/
void CqOcclusionBox::DeleteHierarchy()
{
	// make sure this isn't being called out of turn.
	assert( m_Hierarchy );
	assert( m_LevelStartId );

	delete [] m_Hierarchy;
	m_Hierarchy = NULL;
	delete [] m_LevelStartId;
	m_LevelStartId = NULL;
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

	m_Hierarchy[ 0 ].SetBounds( xMin, yMin, xMax, yMax );
	m_Hierarchy[ 0 ].SetupChildren();
	m_Hierarchy[ 0 ].Clear();
}


//----------------------------------------------------------------------
/** Recursively set the boundaries for this box's children.
 */

void CqOcclusionBox::SetupChildren()
{
	int firstChildId = m_Id * 4 + 1;
	if ( firstChildId >= m_TotalBoxes )
	{
		// bottom of tree. setup the pixels we cover
		for ( int j = m_MinY; j < m_MaxY; j++ )
		{
			for ( int i = m_MinX; i < m_MaxX; i++ )
			{
				CqImagePixel* pie;
				m_Bucket->ImageElement( i, j, pie );
				pie->SetOcclusionBoxId( m_Id );
			}
		}
	}
	else
	{
		TqInt midX = ( m_MaxX - m_MinX ) / 2 + m_MinX;
		TqInt midY = ( m_MaxY - m_MinY ) / 2 + m_MinY;

		m_Hierarchy[ firstChildId ].SetBounds( m_MinX, m_MinY, midX, midY );
		m_Hierarchy[ firstChildId + 1 ].SetBounds( midX, m_MinY, m_MaxX, midY );
		m_Hierarchy[ firstChildId + 2 ].SetBounds( m_MinX, midY, midX, m_MaxY );
		m_Hierarchy[ firstChildId + 3 ].SetBounds( midX, midY, m_MaxX, m_MaxY );

		m_Hierarchy[ firstChildId ].SetupChildren();
		m_Hierarchy[ firstChildId + 1 ].SetupChildren();
		m_Hierarchy[ firstChildId + 2 ].SetupChildren();
		m_Hierarchy[ firstChildId + 3 ].SetupChildren();
	}
}


//----------------------------------------------------------------------
/** Update the stored min and max Z-Values for any boxes at this level
 *   where they might have changed. If we do make changes then we repeat the
 *  process for the parent boxes one level up in the hierarchy. static.
 */

void CqOcclusionBox::UpdateLevel( TqInt level )
{
	assert( level < m_HierarchyLevels && level >= 0 );

	int firstBox = m_LevelStartId[ level ];
	int lastBox = m_LevelStartId[ level + 1 ] - 1;

	TqBool updateNextLevel = TqFalse;
	for ( TqInt i = firstBox; i <= lastBox; i++ )
	{
		// only update if it's been marked as needing it
		if ( m_Hierarchy[ i ].NeedsUpdating() )
		{
			// only mark parent for update if we actually change
			if ( m_Hierarchy[ i ].UpdateZValues() )
			{
				m_Hierarchy[ i / 4 ].MarkForUpdate();
				updateNextLevel = TqTrue;
			}
		}
	}

	// move up a level
	if ( updateNextLevel && level > 0 )
		UpdateLevel( level - 1 );
}


//----------------------------------------------------------------------
/** Set the screen pixel bounds of this box
*/

void CqOcclusionBox::SetBounds( TqInt x0, TqInt y0, TqInt x1, TqInt y1 )
{
	m_MinX = x0;
	m_MinY = y0;
	m_MaxX = x1;
	m_MaxY = y1;
}


//----------------------------------------------------------------------
/** Reset the stored min and max Z values for this box and all its children.
*/

void CqOcclusionBox::Clear()
{
	m_MinZ = FLT_MAX;
	m_MaxZ = FLT_MAX;
	m_NeedsUpdating = TqFalse;

	TqInt firstChildId = m_Id * 4 + 1;
	if ( firstChildId >= m_TotalBoxes )
		return ; // bottom of tree. we have no children

	for ( TqInt i = 0; i < 4; i++ )
		m_Hierarchy[ firstChildId + i ].Clear();
}


//----------------------------------------------------------------------
/** Text if we can safely cull an object with the given bound.
 * \param bound of object to test
 * \return true if the bound lies behind everything in this box
*/

TqBool CqOcclusionBox::IsCullable( CqBound* bound )
{
	CqVector3D vecMin = bound->vecMin();
	CqVector3D vecMax = bound->vecMax();
	TqFloat zVal = vecMin.z();

	if ( zVal > m_MaxZ )
	{
		// bound totally hidden by box
		return TqTrue;
	}
	else if ( zVal < m_MinZ )
	{
		// bound totally visible
		return TqFalse;
	}

	TqInt firstChildId = m_Id * 4 + 1;
	if ( firstChildId >= m_TotalBoxes )
	{
		// no children; check pixels directly
		TqInt x0 = MAX( m_MinX, static_cast<TqInt>( vecMin.x() ) );
		TqInt y0 = MAX( m_MinY, static_cast<TqInt>( vecMin.y() ) );
		TqInt x1 = MIN( m_MaxX, static_cast<TqInt>( vecMax.x() ) );
		TqInt y1 = MIN( m_MaxY, static_cast<TqInt>( vecMax.y() ) );

		for ( TqInt j = y0; j < y1; j++ )
		{
			for ( TqInt i = x0; i < x1; i++ )
			{
				if ( zVal < m_Bucket->MaxDepth( i, j ) )
					return TqFalse;
			}
		}
	}
	else
	{
		// recursively test children. return true if bound is hidden in all children.
		for ( TqInt i = 0; i < 4; i++ )
		{
			if ( m_Hierarchy[ firstChildId + i ].Overlaps( bound ) )
			{
				if ( !m_Hierarchy[ firstChildId + i ].IsCullable( bound ) )
					return TqFalse;
			}
		}
	}
	return TqTrue;
}


//----------------------------------------------------------------------
/** Update the stored min and max Z values for this box.
      Assumes that our child boxes are all up to date.
 * \return true if we changed anthing.
*/
TqBool CqOcclusionBox::UpdateZValues()
{
	TqBool madeUpdate = TqFalse;
	TqFloat currentMaxZ = -FLT_MAX;
	TqFloat currentMinZ = m_MinZ;

	TqInt firstChildId = m_Id * 4 + 1;
	if ( firstChildId >= m_TotalBoxes )
	{
		// we don't have any children so check pixel depths directly
		for ( TqInt j = m_MinY; j < m_MaxY; j++ )
		{
			for ( TqInt i = m_MinX; i < m_MaxX; i++ )
			{
				CqImagePixel* pie;
				m_Bucket->ImageElement( i, j, pie );
				// only update if it's been marked as needing it
				if ( pie->NeedsZUpdating() )
				{
					pie->UpdateZValues();
				}
				if ( pie->MaxDepth() > currentMaxZ )
				{
					currentMaxZ = pie->MaxDepth();
				}
				if ( pie->MinDepth() < currentMinZ )
				{
					currentMinZ = pie->MinDepth();
				}
			}
		}
	}
	else
	{
		for ( TqInt i = 0; i < 4; i++ )
		{
			if ( m_Hierarchy[ firstChildId + i ].m_MaxZ > currentMaxZ )
			{
				currentMaxZ = m_Hierarchy[ firstChildId + i ].m_MaxZ;
			}
			if ( m_Hierarchy[ firstChildId + i ].m_MinZ < currentMinZ )
			{
				currentMinZ = m_Hierarchy[ firstChildId + i ].m_MinZ;
			}
		}
	}

	if ( currentMaxZ < m_MaxZ )
	{
		m_MaxZ = currentMaxZ;
		madeUpdate = TqTrue;
	}
	if ( currentMinZ < m_MinZ )
	{
		m_MinZ = currentMinZ;
		madeUpdate = TqTrue;
	}

	m_NeedsUpdating = TqFalse;
	return madeUpdate;
}


//----------------------------------------------------------------------
/** Tests if the given bound overlaps this box
 * \param bound The bounding box to test
 * \return true if the bound overlaps.
*/

TqBool CqOcclusionBox::Overlaps( CqBound* bound )
{
	TqBool retval = TqTrue;

	if (!( bound->vecMin().x() <= m_MaxX &&
	        bound->vecMin().y() <= m_MaxY &&
	        bound->vecMax().x() >= m_MinX &&
	        bound->vecMax().y() >= m_MinY ))
	{
		retval = TqFalse;
	}

	return retval;
}

#ifdef AQSIS_SYSTEM_MACOSX
#pragma CC_OPT_RESTORE
#endif

END_NAMESPACE( Aqsis )

