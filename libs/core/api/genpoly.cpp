// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Declares a class for handling general polygons with loops.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>
#include	"genpoly.h"
#include	<aqsis/math/math.h>

namespace Aqsis {

//---------------------------------------------------------------------
/** Copy constructor
 */

CqPolygonGeneral2D::CqPolygonGeneral2D( const CqPolygonGeneral2D& From )
{
	*this = From;
}


//---------------------------------------------------------------------
/** Copy operator
 */

CqPolygonGeneral2D& CqPolygonGeneral2D::operator=( const CqPolygonGeneral2D& From )
{
	// Copy the vertices
	TqInt iVertex = From.cVertices();
	m_aiVertices.resize( iVertex );
	while ( iVertex-- > 0 )
		m_aiVertices[ iVertex ] = From.m_aiVertices[ iVertex ];

	m_Orientation = From.m_Orientation;
	m_Axis = From.m_Axis;
	m_Reverse = From.m_Reverse;

	m_pVertices = From.m_pVertices;

	return ( *this );
}

//---------------------------------------------------------------------
void CqPolygonGeneral2D::CalcAxis()
{
	CqParameterTyped<CqVector4D, CqVector3D>* P = m_pVertices->P();
	// Obtain a nondegenerate normal vector for the polygon.
	CqVector3D normal(1,0,0);
	TqInt i = 0;
	TqFloat maxNormalComp = 0;
	while(i+2 < cVertices() && maxNormalComp < 1e-6)
	{
		CqVector3D diff1 = vectorCast<CqVector3D>(P->pValue(m_aiVertices[i+1])[0] - P->pValue(m_aiVertices[i])[0]);
		CqVector3D diff2 = vectorCast<CqVector3D>(P->pValue(m_aiVertices[i+2])[0] - P->pValue(m_aiVertices[i+1])[0]);
		normal = diff1 % diff2;
		// get absolute value of normal componenets.
		normal = Aqsis::max(normal, -normal);
		// maximum component of the normal.
		maxNormalComp = Aqsis::max(Aqsis::max(normal.x(), normal.y()), normal.z());
		++i;
	}
	// We want to project out the axis which has the maximum normal component.
	if(normal.x() > normal.y() && normal.x() > normal.z())
		SetAxis(Axis_YZ);
	else if(normal.y() > normal.x() && normal.y() > normal.z())
		SetAxis(Axis_XZ);
	else
		SetAxis(Axis_XY);
}

//---------------------------------------------------------------------
/** Swap the direction of a polygon.
 */

void CqPolygonGeneral2D::SwapDirection()
{
	TqInt vertices2 = cVertices() / 2;
	TqInt vertices1 = cVertices() - 1;
	for ( TqInt iVertex = 0; iVertex < vertices2; iVertex++ )
	{
		/* Mathematically equivalent */
		TqInt which = vertices1 - iVertex;
		TqInt tmp = m_aiVertices[ iVertex ];

		m_aiVertices[ iVertex ] = m_aiVertices[ which ];
		m_aiVertices[ which ] = tmp;
	}
	CalcOrientation();
	m_Reverse = !m_Reverse;
}


//---------------------------------------------------------------------
/** Calculate and store the orientation of this polygon.
 */

TqInt CqPolygonGeneral2D::CalcOrientation()
{
	// Calculate the area of this polygon, and
	// if it is negative the polygon is clockwise.
	TqInt vertices1 = cVertices() - 1;

	TqFloat	Area = ( *this ) [ vertices1 ].x() * ( *this ) [ 0 ].y() -
	               ( *this ) [ 0 ].x() * ( *this ) [ vertices1 ].y();


	for ( TqInt iVertex = 0; iVertex < vertices1; iVertex++ )
		Area += ( *this ) [ iVertex ].x() * ( *this ) [ iVertex + 1 ].y() -
		        ( *this ) [ iVertex + 1 ].x() * ( *this ) [ iVertex ].y();

	if ( Area >= 0.0 )
		m_Orientation = Orientation_AntiClockwise;
	else
		m_Orientation = Orientation_Clockwise;

	return ( m_Orientation );
}


//---------------------------------------------------------------------
/** Calculate the orientation of the triangle formed using the vertices of this
 *  polygon indexed by the 3 provided indices.
 */

TqInt CqPolygonGeneral2D::CalcDeterminant( TqInt i1, TqInt i2, TqInt i3 ) const
{
	assert( i1 >= 0 && i1 <= cVertices() );
	assert( i2 >= 0 && i2 <= cVertices() );
	assert( i3 >= 0 && i3 <= cVertices() );

	// TODO: Look up what a determinant is and therefore, why this works.
	TqFloat	Determ = ( ( *this ) [ i2 ].x() - ( *this ) [ i1 ].x() )
	                 * ( ( *this ) [ i3 ].y() - ( *this ) [ i1 ].y() )
	                 - ( ( *this ) [ i3 ].x() - ( *this ) [ i1 ].x() )
	                 * ( ( *this ) [ i2 ].y() - ( *this ) [ i1 ].y() );

	if ( Determ > 0.0 )
		return ( Orientation_AntiClockwise );
	else
	{
		if ( Determ == 0.0 )
		{
			return( Orientation_Unknown );
			//if ( ( *this ) [ i1 ] == ( *this ) [ i2 ] ||
			//     ( *this ) [ i1 ] == ( *this ) [ i3 ] ||
			//     ( *this ) [ i2 ] == ( *this ) [ i3 ] )
			//    return ( Orientation_AntiClockwise );
			//else
			//    return ( Orientation_Clockwise );
		}
		else
			return ( Orientation_Clockwise );
	}
	return ( Orientation_Unknown );
}


//---------------------------------------------------------------------
/** Determine whether any of the vertices indexed by the values in iList are within the
 *  triangle formed by the vertices indexed by i1, i2, and i3
 */

bool CqPolygonGeneral2D::NoneInside( TqInt i1, TqInt i2, TqInt i3, std::vector<TqInt>& iList ) const
{
	TqUint iVertex;
	TqUint size = iList.size();
	for ( iVertex = 0; iVertex < size; iVertex++ )
	{
		TqInt iN = iList[ iVertex ];

		// Ignore the vertices which make up the specified triangle
		if ( ( iN == i1 ) || ( iN == i2 ) || ( iN == i3 ) )
			continue;

		// Use the reverse direction of the triangle lines, and if the
		// triangle fromed with the vertex is the same orientation, the
		// vertex lies outside that edge.
		TqInt __t1, __t2, __t3;
		__t1 = CalcDeterminant( i2, i1, iN );
		__t2 = CalcDeterminant( i1, i3, iN );
		__t3 = CalcDeterminant( i3, i2, iN );
		if ( ( __t1 == m_Orientation ) ||
		        ( __t2 == m_Orientation ) ||
		        ( __t3 == m_Orientation ) )
			continue;
		else
		{
			// If it is coincident with one of the vertices, then presume it is inside.
			if ( ( ( *this ) [ iN ] == ( *this ) [ i1 ] ) ||
			        ( ( *this ) [ iN ] == ( *this ) [ i2 ] ) ||
			        ( ( *this ) [ iN ] == ( *this ) [ i3 ] ) )
				continue;
			else
				return ( false );
		}
	}
	return ( true );
}


//---------------------------------------------------------------------
/** Check for any duplicate points in the polygon and remove them.
 */

void CqPolygonGeneral2D::EliminateDuplicatePoints()
{}


//---------------------------------------------------------------------
/** Determine whether the specified polygon is entirely within this one.
 *  Uses AntiClockwise as direction.
 */

bool CqPolygonGeneral2D::Contains( CqPolygonGeneral2D& polyCheck )
{
	assert( polyCheck.cVertices() > 0 && cVertices() > 0 );

	TqInt vertices = polyCheck.cVertices();
	for ( TqInt iVertex = 0; iVertex < vertices; iVertex++ )
	{
		TqInt	c = 0;
		TqFloat	x = polyCheck[ iVertex ].x();
		TqFloat	y = polyCheck[ iVertex ].y();

		// Check if this vertex is inside this polygon.
		TqInt	i, j;
		for ( i = 0, j = vertices - 1; i < vertices; j = i++ )
		{
			// Check if this edge spans the vertex in y
			if ( ( ( ( ( *this ) [ i ].y() <= y ) && ( y < ( *this ) [ j ].y() ) ) ||
			        ( ( ( *this ) [ j ].y() <= y ) && ( y < ( *this ) [ i ].y() ) ) ) &&
			        // and if so, check the position of the vertex in relation to the edge.
			        ( x < ( ( *this ) [ j ].x() - ( *this ) [ i ].x() ) * ( y - ( *this ) [ i ].y() ) /
			          ( ( *this ) [ j ].y() - ( *this ) [ i ].y() ) + ( *this ) [ i ].x() ) )
				c = !c;
		}
		// If this point is outside, then the polygon cannot be entirely inside.
		if ( !c )
			return ( false );
	}
	return ( true );
}


//---------------------------------------------------------------------
/** Combine the two polygons.
 *  Determine the two closest points on the two polygons, and then insert the 
 *  new polygon into the list at this point.  Closing the end afterwards.
 */

void CqPolygonGeneral2D::Combine( CqPolygonGeneral2D& polyFrom )
{
	// Go through and find the two points on the polygons
	// which are closest together.

	CqVector2D	currToPrev, currToNext, minToPrev, minToNext;
	TqInt	iMinThis = 0;
	TqInt	iMinThat = 0;
	TqFloat	CurrDist;
	TqFloat	MinDist = FLT_MAX;

	TqInt i;
	TqInt vertices = cVertices();
	TqInt polyvertices = polyFrom.cVertices();
	for ( i = 0; i < vertices; i++ )
	{
		TqInt j;

		for ( j = 0; j < polyvertices; j++ )
		{
			CqVector2D	vecTemp( ( *this ) [ i ] - polyFrom[ j ] );
			CurrDist = static_cast<TqFloat>( sqrt( vecTemp * vecTemp ) );

			if ( CurrDist == MinDist )
			{
				currToPrev = ( i > 0 ) ? ( *this ) [ i - 1 ] - ( *this ) [ i ] :
				             ( *this ) [ cVertices() - 1 ] - ( *this ) [ i ];
				currToNext = ( i < cVertices() - 1 ) ? ( *this ) [ i + 1 ] - ( *this ) [ i ] :
				             ( *this ) [ 0 ] - ( *this ) [ i ];

				minToPrev = ( iMinThis > 0 ) ? ( *this ) [ iMinThis - 1 ] - ( *this ) [ iMinThis ] :
				            ( *this ) [ cVertices() - 1 ] - ( *this ) [ iMinThis ];
				minToNext = ( iMinThis < cVertices() - 1 ) ? ( *this ) [ iMinThis + 1 ] - ( *this ) [ iMinThis ] :
				            ( *this ) [ 0 ] - ( *this ) [ iMinThis ];

				CqVector2D	vecTest = polyFrom[ j ] - ( *this ) [ i ];

				currToPrev.Unit();
				currToNext.Unit();
				minToPrev.Unit();
				minToNext.Unit();

				vecTemp = currToPrev - vecTest;
				TqFloat	distCP = static_cast<TqFloat>( sqrt( vecTemp * vecTemp ) );
				vecTemp = currToNext - vecTest;
				TqFloat	distCN = static_cast<TqFloat>( sqrt( vecTemp * vecTemp ) );
				vecTemp = minToPrev - vecTest;
				TqFloat	distMP = static_cast<TqFloat>( sqrt( vecTemp * vecTemp ) );
				vecTemp = minToNext - vecTest;
				TqFloat	distMN = static_cast<TqFloat>( sqrt( vecTemp * vecTemp ) );

				if ( ( distCP + distCN ) < ( distMP + distMN ) )
				{
					MinDist = CurrDist;
					iMinThis = i;
					iMinThat = j;
				}
			}
			else
			{
				if ( CurrDist < MinDist )
				{
					MinDist = CurrDist;
					iMinThis = i;
					iMinThat = j;
				}
			}
		}
	}
	// Now combine the two polygons at the closest points.

	std::vector<TqInt>	avecNew;

	// First copy the vertices from this one, from the min point up to the end...
	for ( i = iMinThis; i < vertices; i++ )
		avecNew.push_back( m_aiVertices[ i ] );

	// ...then copy the vertices from this one, from 0 up to (and including) the min point...
	for ( i = 0; i <= iMinThis; i++ )
		avecNew.push_back( m_aiVertices[ i ] );

	// ...then copy the vertices from that one, from the min point up to the end...
	for ( i = iMinThat; i < polyvertices; i++ )
		avecNew.push_back( polyFrom.m_aiVertices[ i ] );

	// ...then copy the vertices from that one, from 0 up to (and including) the min point...
	for ( i = 0; i <= iMinThat; i++ )
		avecNew.push_back( polyFrom.m_aiVertices[ i ] );

	// Now copy the new list of vertices to this new polygon.
	TqUint size = (TqUint) avecNew.size();
	m_aiVertices.resize( size );
	TqUint ivert;
	for ( ivert = 0; ivert < size; ivert++ )
		m_aiVertices[ ivert ] = avecNew[ ivert ];
}


//---------------------------------------------------------------------
/** Return a list of triangles which cove the surface of this general polygon.
 */

void CqPolygonGeneral2D::Triangulate( std::vector<TqInt>& aiList ) const
{
	// This is done by checking each vertex in turn to see if it can successfully be chopped off.
	// If at the end there are more than 3 vertices left which cannot be chopped off, the
	// polygon is self intersecting.

	std::vector<TqInt>	iList;
	TqInt size = m_aiVertices.size() ;
	iList.resize( size );
	TqInt iVertex = size;
	while ( iVertex-- > 0 )
		iList[ iVertex ] = iVertex;

	TqInt cVertex = size;
	while ( cVertex > 3 )
	{
		bool	fDone = false;
		TqInt	iPrev = cVertex - 1;
		TqInt	iCurr = 0;
		TqInt	iNext = 1;

		while ( ( iCurr < cVertex ) && ( fDone == false ) )
		{
			iPrev = iCurr - 1;
			iNext = iCurr + 1;

			if ( iCurr == 0 )
				iPrev = cVertex - 1;
			else
				if ( iCurr == cVertex - 1 )
					iNext = 0;

			TqInt	CurrDeterm = CalcDeterminant( iList[ iPrev ],
			                                    iList[ iCurr ],
			                                    iList[ iNext ] );
			TqInt	CurrPos = NoneInside( iList[ iPrev ],
			                            iList[ iCurr ],
			                            iList[ iNext ],
			                            iList );
			if ( ( CurrDeterm == Orientation() ) &&
			        ( CurrPos != 0 ) )
				fDone = true;
			else
				iCurr++;
		}

		if ( fDone == false )
			return ;
		else
		{
			if( m_Reverse )
			{
				aiList.push_back( m_aiVertices[ iList[ iNext ] ] );
				aiList.push_back( m_aiVertices[ iList[ iCurr ] ] );
				aiList.push_back( m_aiVertices[ iList[ iPrev ] ] );
			}
			else
			{
				aiList.push_back( m_aiVertices[ iList[ iPrev ] ] );
				aiList.push_back( m_aiVertices[ iList[ iCurr ] ] );
				aiList.push_back( m_aiVertices[ iList[ iNext ] ] );
			}

			cVertex -= 1;
			for ( iVertex = iCurr; iVertex < cVertex; iVertex++ )
				iList[ iVertex ] = iList[ iVertex + 1 ];
			iList.resize( cVertex );
		}
	}
	if( cVertex == 3 )
	{
		if( m_Reverse )
		{
			aiList.push_back( m_aiVertices[ iList[ 2 ] ] );
			aiList.push_back( m_aiVertices[ iList[ 1 ] ] );
			aiList.push_back( m_aiVertices[ iList[ 0 ] ] );
		}
		else
		{
			aiList.push_back( m_aiVertices[ iList[ 0 ] ] );
			aiList.push_back( m_aiVertices[ iList[ 1 ] ] );
			aiList.push_back( m_aiVertices[ iList[ 2 ] ] );
		}
	}
}


//---------------------------------------------------------------------

} // namespace Aqsis
