// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Implements classes for the subdivision surface GPrim.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<math.h>

#include	"aqsis.h"
#include	"subdivision.h"
#include	"renderer.h"
#include	"micropolygon.h"
#include	"imagebuffer.h"
#include	"polygon.h"

START_NAMESPACE( Aqsis )

DEFINE_STATIC_MEMORYPOOL( CqWEdge );
DEFINE_STATIC_MEMORYPOOL( CqWFace );
DEFINE_STATIC_MEMORYPOOL( CqWVert );

//---------------------------------------------------------------------
/** Remove an edge reference from the array.
 */

void CqWVert::RemoveEdge( CqWEdge* pE )
{
	for ( std::vector<CqWEdge*>::iterator i = m_apEdges.begin(); i != m_apEdges.end(); i++ )
	{
		if ( ( *i ) == pE )
		{
			m_apEdges.erase( i );
			return ;
		}
	}
	assert( false );
}


//---------------------------------------------------------------------
/** Is this edge on the boundary of the mesh?
 */

TqBool CqWEdge::IsBoundary()
{
	if ( ( !pfLeft() || !pfRight() ) && !( !pfLeft() && !pfRight() ) )
		return ( TqTrue );
	else
		return ( TqFalse );
}


//---------------------------------------------------------------------
/** Is this edge valid? i.e. has it been used by at least one face.
 */

TqBool CqWEdge::IsValid()
{
	if ( !pfLeft() && !pfRight() )
		return ( TqFalse );
	else
		return ( TqTrue );
}


//---------------------------------------------------------------------
/** Create a new WVertex for this edge taking into account its sharpness, and that of its neighbours.
 */

CqWVert* CqWEdge::CreateSubdividePoint( CqSubdivider* pSurf, CqPolygonPoints* pPoints, CqWVert* pV, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
                                        TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	TqUint index = pV->iVertex();
	m_pvSubdivide = pV;

	CqVector3D P = CreateSubdivideScalar( P, &CqSubdivider::SubdP, pSurf, pPoints );
	if ( pPoints->P().Size() <= index ) pPoints->P().SetSize( index + 1 );
	pPoints->P() [ index ] = P;

	if ( uses_s && has_s )
	{
		TqFloat s = CreateSubdivideScalar( s, &CqSubdivider::Subds, pSurf, pPoints );
		if ( pPoints->s().Size() <= index ) pPoints->s().SetSize( index + 1 );
		pPoints->s() [ index ] = s;
	}

	if ( uses_t && has_t )
	{
		TqFloat t = CreateSubdivideScalar( t, &CqSubdivider::Subdt, pSurf, pPoints );
		if ( pPoints->t().Size() <= index ) pPoints->t().SetSize( index + 1 );
		pPoints->t() [ index ] = t;
	}

	if ( uses_Cs && has_Cs )
	{
		CqColor Cq = CreateSubdivideScalar( Cq, &CqSubdivider::SubdCs, pSurf, pPoints );
		if ( pPoints->Cs().Size() <= index ) pPoints->Cs().SetSize( index + 1 );
		pPoints->Cs() [ index ] = Cq;
	}

	if ( uses_Os && has_Os )
	{
		CqColor Os = CreateSubdivideScalar( Os, &CqSubdivider::SubdOs, pSurf, pPoints );
		if ( pPoints->Os().Size() <= index ) pPoints->Os().SetSize( index + 1 );
		pPoints->Os() [ index ] = Os;
	}

	return ( pV );
}



//---------------------------------------------------------------------
/** Subdivide this edge into two subedges, can only be called AFTER CreateSubdividePoint.
 */

void CqWEdge::Subdivide( CqSubdivider* pSurf )
{
	if ( m_pvSubdivide == NULL )
	{
		//throw("Error : Attempting to split an edge with no midpoint");
		return ;
	}

	m_peHeadHalf = new CqWEdge();
	m_peTailHalf = new CqWEdge();
	pSurf->AddEdge( m_peHeadHalf );
	pSurf->AddEdge( m_peTailHalf );

	m_peHeadHalf->SetpvHead( pvHead() );
	m_peHeadHalf->SetpvTail( m_pvSubdivide );
	m_peTailHalf->SetpvHead( m_pvSubdivide );
	m_peTailHalf->SetpvTail( pvTail() );

	m_peHeadHalf->SetpeHeadRight( peHeadRight() );
	m_peHeadHalf->SetpeHeadLeft( peHeadLeft() );

	m_peTailHalf->SetpeTailRight( peTailRight() );
	m_peTailHalf->SetpeTailLeft( peTailLeft() );

	// Calculate the new sharpness values.
	if ( m_Sharpness == 0 )
		m_peHeadHalf->m_Sharpness = m_peTailHalf->m_Sharpness = 0;
	else
	{
		// Find the edges which make up this crease.
		TqInt i = 0;
		CqWEdge* peA = pvHead() ->lEdges() [ i ];
		while ( peA->m_Sharpness <= 0 && i < pvHead() ->cEdges() )
			peA = pvHead() ->lEdges() [ i++ ];

		i = 0;
		CqWEdge* peC = pvTail() ->lEdges() [ i ];
		while ( peC->m_Sharpness <= 0 && i < pvTail() ->cEdges() )
			peC = pvTail() ->lEdges() [ i++ ];

		m_peHeadHalf->m_Sharpness = ( ( peA->m_Sharpness + ( 3.0f * m_Sharpness ) ) / 4.0f ) - 1.0f;
		m_peTailHalf->m_Sharpness = ( ( ( 3.0f * m_Sharpness ) + peC->m_Sharpness ) / 4.0f ) - 1.0f;

		if ( m_peHeadHalf->m_Sharpness < 0 ) m_peHeadHalf->m_Sharpness = 0.0;
		if ( m_peTailHalf->m_Sharpness < 0 ) m_peTailHalf->m_Sharpness = 0.0;
	}

	m_pvSubdivide->AddEdge( peHeadHalf() );
	m_pvSubdivide->AddEdge( peTailHalf() );
	pvHead() ->AddEdge( peHeadHalf() );
	pvTail() ->AddEdge( peTailHalf() );
}


//---------------------------------------------------------------------
/** Create a new WVertex as the centroid of this face.
 */

CqWVert* CqWFace::CreateSubdividePoint( CqSubdivider* pSurf, CqPolygonPoints* pPoints, CqWVert* pV, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
                                        TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	TqUint index = pV->iVertex();
	m_pvSubdivide = pV;

	CqVector3D P = CreateSubdivideScalar( P, &CqSubdivider::SubdP, pSurf, pPoints );
	if ( pPoints->P().Size() <= index ) pPoints->P().SetSize( index + 1 );
	pPoints->P() [ index ] = P;

	if ( uses_s && has_s )
	{
		TqFloat s = CreateSubdivideScalar( s, &CqSubdivider::Subds, pSurf, pPoints );
		if ( pPoints->s().Size() <= index ) pPoints->s().SetSize( index + 1 );
		pPoints->s() [ index ] = s;
	}

	if ( uses_t && has_t )
	{
		TqFloat t = CreateSubdivideScalar( t, &CqSubdivider::Subdt, pSurf, pPoints );
		if ( pPoints->t().Size() <= index ) pPoints->t().SetSize( index + 1 );
		pPoints->t() [ index ] = t;
	}

	if ( uses_Cs && has_Cs )
	{
		CqColor Cq = CreateSubdivideScalar( Cq, &CqSubdivider::SubdCs, pSurf, pPoints );
		if ( pPoints->Cs().Size() <= index ) pPoints->Cs().SetSize( index + 1 );
		pPoints->Cs() [ index ] = Cq;
	}

	if ( uses_Os && has_Os )
	{
		CqColor Os = CreateSubdivideScalar( Os, &CqSubdivider::SubdOs, pSurf, pPoints );
		if ( pPoints->Os().Size() <= index ) pPoints->Os().SetSize( index + 1 );
		pPoints->Os() [ index ] = Os;
	}

	return ( pV );
}

//---------------------------------------------------------------------
/** Find the specified edge.
 */

CqWEdge* CqSubdivider::FindEdge( CqWEdge* pE )
{
	TqInt i;

	// If no vertices or no edges, cannot have been constructed yet.
	if ( m_apVerts.size() == 0 || m_apEdges.size() == 0 ) return ( NULL );

	for ( i = 0; i < pE->pvHead() ->cEdges(); i++ )
	{
		if ( pE->pvHead() ->pEdge( i ) ->pvTail() == pE->pvTail() ||
		        pE->pvHead() ->pEdge( i ) ->pvHead() == pE->pvTail() )
			return ( pE->pvHead() ->lEdges() [ i ] );
	}

	for ( i = 0; i < pE->pvTail() ->cEdges(); i++ )
	{
		if ( pE->pvTail() ->pEdge( i ) ->pvHead() == pE->pvHead() ||
		        pE->pvTail() ->pEdge( i ) ->pvTail() == pE->pvHead() )
			return ( pE->pvTail() ->lEdges() [ i ] );
	}
	return ( NULL );
}


//---------------------------------------------------------------------
/** Find the specified vertex.
 */

CqWVert* CqSubdivider::FindVertex( CqPolygonPoints* pPoints, const CqVector3D& V )
{
	// If no vertices or no edges, cannot have been constructed yet.
	if ( m_apVerts.size() == 0 || m_apEdges.size() == 0 ) return ( NULL );

	for ( std::vector<CqWVert*>::iterator i = m_apVerts.begin(); i != m_apVerts.end(); i++ )
	{
		if ( pPoints->P() [ ( *i ) ->iVertex() ] == V )
			return ( *i );
	}
	return ( NULL );
}


//---------------------------------------------------------------------
/** Add and edge to the list.
 */

CqWEdge* CqSubdivider::AddEdge( CqWVert* pA, CqWVert* pB )
{
	CqWEdge * pExist;
	CqWEdge eTemp( pA, pB );
	if ( ( pExist = FindEdge( &eTemp ) ) != 0 )
	{
		if ( pExist->pvHead() == pA )
		{
			CqBasicError( ErrorID_NonmanifoldSubdivision, Severity_Fatal, "Subdivision Mesh contains non-manifold data" );
			return ( 0 );
		}
		return ( pExist );
	}
	else
	{
		CqWEdge* pNew = new CqWEdge( pA, pB );
		// TODO: Check if it is valid to return an edge in the opposite direction.
		m_apEdges.push_back( pNew );
		pNew->pvHead() ->AddEdge( pNew );
		pNew->pvTail() ->AddEdge( pNew );
		return ( pNew );
	}
}


//---------------------------------------------------------------------
/** Add a polygon, passed as an array of edge references.
 */

CqWFace* CqSubdivider::AddFace( CqWEdge** pE, TqInt cE )
{
	TqInt i;
	CqWEdge* pCurrE;
	CqWFace* pNewFace = new CqWFace();
	for ( i = 0; i < cE; i++ )
	{
		if ( ( pCurrE = FindEdge( pE[ i ] ) ) == 0 )
		{
			// Error, edges must be available.
			delete( pNewFace );
			return ( 0 );
		}
		else
		{
			if ( pCurrE->bComplete() )
			{
				CqBasicError( ErrorID_NonmanifoldSubdivision, Severity_Fatal, "Subdivision Mesh contains non-manifold data" );
				delete( pNewFace );
				return ( 0 );
			}
			else if ( pCurrE->cFaces() == 0 )
				pCurrE->SetpfLeft( pNewFace );
			else
				pCurrE->SetpfRight( pNewFace );

			pCurrE->IncrFaces();
		}
		pNewFace->AddEdge( pCurrE );
	}

	// Fill in the wing data
	for ( i = 0; i < pNewFace->cEdges(); i++ )
	{
		if ( pNewFace->pEdge( i ) ->pfLeft() == pNewFace )
		{
			pNewFace->pEdge( i ) ->SetpeTailLeft( ( i == pNewFace->cEdges() - 1 ) ? pNewFace->pEdge( 0 ) : pNewFace->pEdge( i + 1 ) );
			pNewFace->pEdge( i ) ->SetpeHeadLeft( ( i == 0 ) ? pNewFace->pEdge( pNewFace->cEdges() - 1 ) : pNewFace->pEdge( i - 1 ) );
		}
		else
		{
			pNewFace->pEdge( i ) ->SetpeHeadRight( ( i == pNewFace->cEdges() - 1 ) ? pNewFace->pEdge( 0 ) : pNewFace->pEdge( i + 1 ) );
			pNewFace->pEdge( i ) ->SetpeTailRight( ( i == 0 ) ? pNewFace->pEdge( pNewFace->cEdges() - 1 ) : pNewFace->pEdge( i - 1 ) );
		}
	}
	m_apFaces.push_back( pNewFace );
	return ( pNewFace );
}



//---------------------------------------------------------------------
/** Destructor
 */

CqSubdivider::~CqSubdivider()
{
	// Delete all edges, vertices and faces.
	TqUint i;
	for ( i = 0; i < m_apVerts.size(); i++ )
		delete( m_apVerts[ i ] );

	for ( i = 0; i < m_apFaces.size(); i++ )
		delete( m_apFaces[ i ] );

	for ( i = 0; i < m_apEdges.size(); i++ )
		delete( m_apEdges[ i ] );
}


//---------------------------------------------------------------------
/** Subdivide the surface, using Catmull Clark subdivision rules.
 */

void CqSubdivider::Subdivide()
{
	TqInt i;
	static CqVector3D vecT;

	TqInt lUses = Uses();
	TqBool uses_s = USES( lUses, EnvVars_s );
	TqBool uses_t = USES( lUses, EnvVars_t );
	TqBool uses_Cs = USES( lUses, EnvVars_Cs );
	TqBool uses_Os = USES( lUses, EnvVars_Os );

	TqBool has_s = bHass();
	TqBool has_t = bHast();
	TqBool has_Cs = bHasCs();
	TqBool has_Os = bHasOs();

	// Create an array big enough to hold all the additional points to be created.
	TqInt newcVerts = cVerts();
	TqInt oldcVerts = newcVerts;
	newcVerts += cFaces();
	newcVerts += cEdges();

	m_apVerts.reserve( cVerts() + cFaces() + cEdges() );
	m_apFaces.reserve( cFaces() + ( cFaces() * 4 ) );
	m_apEdges.reserve( cEdges() + ( cEdges() * 2 ) );

	TqInt index = oldcVerts;

	TqInt ifT = cFaces();
	TqInt ieT = cEdges();

	// Create face points.
	CreateFacePoints( index, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	// Create edge points.
	CreateEdgePoints( index, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	// Smooth vertex points
	SmoothVertexPoints( oldcVerts, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );

	// Create new edges.
	for ( i = 0; i < ieT; i++ )
		pEdge( i ) ->Subdivide( this );

	// Create new faces
	std::vector<CqWEdge*> aEdges;
	for ( i = 0; i < ifT; i++ )
	{
		TqInt j;
		// Build all internal edges, external subdivided edges are built by WEdge structures themselves.
		CqWReference grE( pFace( i ) ->pEdge( 0 ), pFace( i ) );
		CqWReference grE2;
		aEdges.resize( pFace( i ) ->cEdges() );
		for ( j = 0; j < pFace( i ) ->cEdges(); j++ )
		{
			aEdges[ j ] = new CqWEdge;
			aEdges[ j ] ->SetpvHead( pFace( i ) ->pvSubdivide() );
			aEdges[ j ] ->SetpvTail( grE.peCurrent() ->pvSubdivide() );
			AddEdge( aEdges[ j ] );

			pFace( i ) ->pvSubdivide() ->AddEdge( aEdges[ j ] );
			grE.peCurrent() ->pvSubdivide() ->AddEdge( aEdges[ j ] );

			grE.peNext();
		}

		// Now build the faces, building the wings as we go.
		grE.Reset( pFace( i ) ->pEdge( 0 ), pFace( i ) );
		grE2.Reset( grE.peCurrent(), pFace( i ) );
		// Point another intelligent reference to the next external edge.
		grE2.pePrev();
		CqWEdge* peI2;
		for ( j = 0; j < pFace( i ) ->cEdges(); j++ )
		{
			// Find the next internal edge.
			if ( j == 0 ) peI2 = aEdges[ pFace( i ) ->cEdges() - 1 ];
			else	peI2 = aEdges[ j - 1 ];

			// Create a new face to be filled in.
			CqWFace* pfNew = new CqWFace;

			// Add the edges and set the edge facet references.
			pfNew->SetQuad( grE.peHeadHalf(), aEdges[ j ], peI2, grE2.peTailHalf() );
			grE.SetpfHeadLeft( pfNew );
			aEdges[ j ] ->SetpfRight( pfNew );
			peI2->SetpfLeft( pfNew );
			grE2.SetpfTailLeft( pfNew );
			// Set up wing data for the edges.
			grE.SetpeHeadTailLeft( aEdges[ j ] );	grE.SetpeHeadHeadLeft( grE2.peTailHalf() );
			aEdges[ j ] ->SetpeTailRight( grE.peHeadHalf() );	aEdges[ j ] ->SetpeHeadRight( peI2 );
			grE2.SetpeTailHeadLeft( peI2 );	grE2.SetpeTailTailLeft( grE.peHeadHalf() );
			peI2->SetpeTailLeft( grE2.peTailHalf() );	peI2->SetpeHeadLeft( aEdges[ j ] );
			// Add the face.
			AddFace( pfNew );

			grE.peNext();
			grE2.peNext();
		}
	}

	for ( i = 0; i < ifT; i++ )
		delete( pFace( i ) );

	for ( i = 0; i < ieT; i++ )
	{
		pEdge( i ) ->pvHead() ->RemoveEdge( pEdge( i ) );
		pEdge( i ) ->pvTail() ->RemoveEdge( pEdge( i ) );
		delete( pEdge( i ) );
	}
	m_apFaces.erase( m_apFaces.begin(), m_apFaces.begin() + ifT );
	m_apEdges.erase( m_apEdges.begin(), m_apEdges.begin() + ieT );
}


//---------------------------------------------------------------------
/** Subdivide the surface, using Catmull Clark subdivision rules.
 * This is a specialised version for use during dicing, makes some assumptions.
 * Must be a quad based patch, i.e. at least one split if more that 4 points.
 * Produces the vertices with indexes usable for creating a MP grid.
 */

void CqSubdivider::DiceSubdivide()
{
	//assert(pFace(0)->cEdges()==4);
	TqInt i;
	static CqVector3D vecT;

	TqInt lUses = Uses();
	TqBool uses_s = USES( lUses, EnvVars_s );
	TqBool uses_t = USES( lUses, EnvVars_t );
	TqBool uses_Cs = USES( lUses, EnvVars_Cs );
	TqBool uses_Os = USES( lUses, EnvVars_Os );

	TqBool has_s = bHass();
	TqBool has_t = bHast();
	TqBool has_Cs = bHasCs();
	TqBool has_Os = bHasOs();

	// NOTE: Not entirely happy about this method, would prefer a more efficient approach!
	// Must create this array here, to ensure we only store the old points, not the subdivided ones.
	std::vector<CqVector3D> aVertices;
	aVertices.resize( cVerts() );

	// Create an array big enough to hold all the additional points to be created.
	TqInt newcVerts = cVerts();
	TqInt oldcVerts = newcVerts;
	newcVerts += cFaces();
	newcVerts += cEdges();

	m_apVerts.reserve( cVerts() + cFaces() + cEdges() );
	m_apFaces.reserve( cFaces() * 4 );
	m_apEdges.reserve( cEdges() * 2 );

	TqInt index = oldcVerts;

	// Keep a count of faces and edges to remove them after subdivision.
	TqInt ifT = cFaces();
	TqInt ieT = cEdges();

	// Create face points.
	CreateFacePoints( index, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	// Create edge points.
	CreateEdgePoints( index, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	// Smooth vertex points
	SmoothVertexPoints( oldcVerts, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );


	// Create new edges.
	for ( i = 0; i < ieT; i++ )
		pEdge( i ) ->Subdivide( this );

	// Create new faces
	std::vector<CqWEdge*> aEdges;
	for ( i = 0; i < ifT; i++ )
	{
		TqInt j;
		// Build all internal edges, external subdivided edges are built by WEdge structures themselves.
		CqWReference grE( pFace( i ) ->pEdge( 0 ), pFace( i ) );
		CqWReference grE2;
		aEdges.reserve( pFace( i ) ->cEdges() );
		for ( j = 0; j < pFace( i ) ->cEdges(); j++ )
		{
			aEdges[ j ] = new CqWEdge;
			aEdges[ j ] ->SetpvHead( pFace( i ) ->pvSubdivide() );
			aEdges[ j ] ->SetpvTail( grE.peCurrent() ->pvSubdivide() );
			AddEdge( aEdges[ j ] );

			pFace( i ) ->pvSubdivide() ->AddEdge( aEdges[ j ] );
			grE.peCurrent() ->pvSubdivide() ->AddEdge( aEdges[ j ] );

			grE.peNext();
		}

		// Now build the faces, building the wings as we go.
		grE.Reset( pFace( i ) ->pEdge( 0 ), pFace( i ) );
		// Point another intelligent reference to the next external edge.
		grE2.Reset( pFace( i ) ->pEdge( 0 ), pFace( i ) );
		grE2.pePrev();

		CqWEdge* peI1, *peI2;
		CqWFace* pfNew;

		// Face 1
		peI1 = aEdges[ 0 ];
		peI2 = aEdges[ pFace( i ) ->cEdges() - 1 ];
		pfNew = new CqWFace;
		// Add the edges and set the edge facet references.
		pfNew->SetQuad( grE.peHeadHalf(), peI1, peI2, grE2.peTailHalf() );
		grE.SetpfHeadLeft( pfNew );
		peI1->SetpfRight( pfNew );
		peI2->SetpfLeft( pfNew );
		grE2.SetpfTailLeft( pfNew );
		// Set up wing data for the edges.
		grE.SetpeHeadTailLeft( peI1 );	grE.SetpeHeadHeadLeft( grE2.peTailHalf() );
		peI1->SetpeTailRight( grE.peHeadHalf() );	peI1->SetpeHeadRight( peI2 );
		grE2.SetpeTailHeadLeft( peI2 );	grE2.SetpeTailTailLeft( grE.peHeadHalf() );
		peI2->SetpeTailLeft( grE2.peTailHalf() );	peI2->SetpeHeadLeft( peI1 );
		// Add the face.
		AddFace( pfNew );
		grE.peNext();
		grE2.peNext();

		// Face 2
		peI1 = aEdges[ 1 ];
		peI2 = aEdges[ 0 ];
		pfNew = new CqWFace;
		// Add the edges and set the edge facet references.
		pfNew->SetQuad( grE2.peTailHalf(), grE.peHeadHalf(), peI1, peI2 );
		grE2.SetpfTailLeft( pfNew );
		grE.SetpfHeadLeft( pfNew );
		peI1->SetpfRight( pfNew );
		peI2->SetpfLeft( pfNew );
		// Set up wing data for the edges.
		grE.SetpeHeadTailLeft( peI1 );	grE.SetpeHeadHeadLeft( grE2.peTailHalf() );
		peI1->SetpeTailRight( grE.peHeadHalf() );	peI1->SetpeHeadRight( peI2 );
		grE2.SetpeTailHeadLeft( peI2 );	grE2.SetpeTailTailLeft( grE.peHeadHalf() );
		peI2->SetpeTailLeft( grE2.peTailHalf() );	peI2->SetpeHeadLeft( peI1 );
		// Add the face.
		AddFace( pfNew );
		grE.peNext();
		grE2.peNext();

		// Face 3
		peI1 = aEdges[ 2 ];
		peI2 = aEdges[ 1 ];
		pfNew = new CqWFace;
		// Add the edges and set the edge facet references.
		pfNew->SetQuad( peI2, grE2.peTailHalf(), grE.peHeadHalf(), peI1 );
		peI2->SetpfLeft( pfNew );
		grE2.SetpfTailLeft( pfNew );
		grE.SetpfHeadLeft( pfNew );
		peI1->SetpfRight( pfNew );
		// Set up wing data for the edges.
		grE.SetpeHeadTailLeft( peI1 );	grE.SetpeHeadHeadLeft( grE2.peTailHalf() );
		peI1->SetpeTailRight( grE.peHeadHalf() );	peI1->SetpeHeadRight( peI2 );
		grE2.SetpeTailHeadLeft( peI2 );	grE2.SetpeTailTailLeft( grE.peHeadHalf() );
		peI2->SetpeTailLeft( grE2.peTailHalf() );	peI2->SetpeHeadLeft( peI1 );
		// Add the face.
		AddFace( pfNew );
		grE.peNext();
		grE2.peNext();

		// Face 4
		peI1 = aEdges[ 3 ];
		peI2 = aEdges[ 2 ];
		pfNew = new CqWFace;
		// Add the edges and set the edge facet references.
		pfNew->SetQuad( peI1, peI2, grE2.peTailHalf(), grE.peHeadHalf() );
		peI1->SetpfRight( pfNew );
		peI2->SetpfLeft( pfNew );
		grE2.SetpfTailLeft( pfNew );
		grE.SetpfHeadLeft( pfNew );
		// Set up wing data for the edges.
		grE.SetpeHeadTailLeft( peI1 );	grE.SetpeHeadHeadLeft( grE2.peTailHalf() );
		peI1->SetpeTailRight( grE.peHeadHalf() );	peI1->SetpeHeadRight( peI2 );
		grE2.SetpeTailHeadLeft( peI2 );	grE2.SetpeTailTailLeft( grE.peHeadHalf() );
		peI2->SetpeTailLeft( grE2.peTailHalf() );	peI2->SetpeHeadLeft( peI1 );
		// Add the face.
		AddFace( pfNew );
	}

	for ( i = 0; i < ifT; i++ )
		delete( pFace( i ) );

	for ( i = 0; i < ieT; i++ )
	{
		pEdge( i ) ->pvHead() ->RemoveEdge( pEdge( i ) );
		pEdge( i ) ->pvTail() ->RemoveEdge( pEdge( i ) );
		delete( pEdge( i ) );
	}
	m_apFaces.erase( m_apFaces.begin(), m_apFaces.begin() + ifT );
	m_apEdges.erase( m_apEdges.begin(), m_apEdges.begin() + ieT );
}



void CqSubdivider::StoreDice( TqInt Level, TqInt& iFace, CqPolygonPoints* pPoints,
                              TqInt uOff, TqInt vOff, TqInt cuv, CqMicroPolyGrid* pGrid,
                              TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
                              TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	CqWFace * pF;
	CqWReference rE;

	if ( Level > 1 )
		StoreDice( Level - 1, iFace, pPoints, uOff, vOff, cuv, pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	else
	{
		pF = pFace( iFace++ );
		rE.Reset( pF->pEdge( 0 ), pF );
		TqInt ivA = rE.pvHead() ->iVertex();
		TqInt ivB = rE.peNext().pvHead() ->iVertex();
		TqInt ivC = rE.peNext().pvHead() ->iVertex();
		TqInt ivD = rE.peNext().pvHead() ->iVertex();

		pGrid->P()->SetValue( ( ( vOff ) * cuv ) + uOff, CqVMStackEntry( SubdP( pPoints, ivA ) ) );
		pGrid->P()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( SubdP( pPoints, ivB ) ) );
		pGrid->P()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdP( pPoints, ivC ) ) );
		pGrid->P()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdP( pPoints, ivD ) ) );

		if ( uses_s && has_s )
		{
			pGrid->s()->SetValue( ( ( vOff ) * cuv ) + uOff, CqVMStackEntry( Subds( pPoints, ivA ) ) );
			pGrid->s()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( Subds( pPoints, ivB ) ) );
			pGrid->s()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( Subds( pPoints, ivC ) ) );
			pGrid->s()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( Subds( pPoints, ivD ) ) );
		}
		if ( uses_t && has_t )
		{
			pGrid->t()->SetValue( ( ( vOff ) * cuv ) + uOff, CqVMStackEntry( Subdt( pPoints, ivA ) ) );
			pGrid->t()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( Subdt( pPoints, ivB ) ) );
			pGrid->t()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( Subdt( pPoints, ivC ) ) );
			pGrid->t()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( Subdt( pPoints, ivD ) ) );
		}
		if ( uses_Cs && has_Cs )
		{
			pGrid->Cs()->SetValue( ( ( vOff ) * cuv ) + uOff, CqVMStackEntry( SubdCs( pPoints, ivA ) ) );
			pGrid->Cs()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( SubdCs( pPoints, ivB ) ) );
			pGrid->Cs()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdCs( pPoints, ivC ) ) );
			pGrid->Cs()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdCs( pPoints, ivD ) ) );
		}
		if ( uses_Os && has_Os )
		{
			pGrid->Os()->SetValue( ( ( vOff ) * cuv ) + uOff, CqVMStackEntry( SubdOs( pPoints, ivA ) ) );
			pGrid->Os()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( SubdOs( pPoints, ivB ) ) );
			pGrid->Os()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdOs( pPoints, ivC ) ) );
			pGrid->Os()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdOs( pPoints, ivD ) ) );
		}
	}

	uOff += 1 << ( Level - 1 );
	if ( Level > 1 )
		StoreDice( Level - 1, iFace, pPoints, uOff, vOff, cuv, pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	else
	{
		pF = pFace( iFace++ );
		rE.Reset( pF->pEdge( 1 ), pF );
		TqInt ivB = rE.pvHead() ->iVertex();
		TqInt ivC = rE.peNext().pvHead() ->iVertex();

		pGrid->P()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( SubdP( pPoints, ivB ) ) );
		pGrid->P()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdP( pPoints, ivC ) ) );

		if ( uses_s && has_s )
		{
			pGrid->s()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( Subds( pPoints, ivB ) ) );
			pGrid->s()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( Subds( pPoints, ivC ) ) );
		}
		if ( uses_t && has_t )
		{
			pGrid->t()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( Subdt( pPoints, ivB ) ) );
			pGrid->t()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( Subdt( pPoints, ivC ) ) );
		}
		if ( uses_Cs && has_Cs )
		{
			pGrid->Cs()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( SubdCs( pPoints, ivB ) ) );
			pGrid->Cs()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdCs( pPoints, ivC ) ) );
		}
		if ( uses_Os && has_Os )
		{
			pGrid->Os()->SetValue( ( ( vOff ) * cuv ) + uOff + 1, CqVMStackEntry( SubdOs( pPoints, ivB ) ) );
			pGrid->Os()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdOs( pPoints, ivC ) ) );
		}
	}

	vOff += 1 << ( Level - 1 );
	if ( Level > 1 )
		StoreDice( Level - 1, iFace, pPoints, uOff, vOff, cuv, pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	else
	{
		pF = pFace( iFace++ );
		rE.Reset( pF->pEdge( 2 ), pF );
		TqInt ivC = rE.pvHead() ->iVertex();
		TqInt ivD = rE.peNext().pvHead() ->iVertex();

		pGrid->P()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdP( pPoints, ivC ) ) );
		pGrid->P()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdP( pPoints, ivD ) ) );

		if ( uses_s && has_s )
		{
			pGrid->s()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( Subds( pPoints, ivC ) ) );
			pGrid->s()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( Subds( pPoints, ivD ) ) );
		}
		if ( uses_t && has_t )
		{
			pGrid->t()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( Subdt( pPoints, ivC ) ) );
			pGrid->t()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( Subdt( pPoints, ivD ) ) );
		}
		if ( uses_Cs && has_Cs )
		{
			pGrid->Cs()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdCs( pPoints, ivC ) ) );
			pGrid->Cs()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdCs( pPoints, ivD ) ) );
		}
		if ( uses_Os && has_Os )
		{
			pGrid->Os()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff + 1, CqVMStackEntry( SubdOs( pPoints, ivC ) ) );
			pGrid->Os()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdOs( pPoints, ivD ) ) );
		}
	}

	uOff -= 1 << ( Level - 1 );
	if ( Level > 1 )
		StoreDice( Level - 1, iFace, pPoints, uOff, vOff, cuv, pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	else
	{
		pF = pFace( iFace++ );
		rE.Reset( pF->pEdge( 3 ), pF );
		TqInt ivD = rE.pvHead() ->iVertex();

		pGrid->P()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdP( pPoints, ivD ) ) );
		if ( uses_s && has_s ) pGrid->s()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( Subds( pPoints, ivD ) ) );
		if ( uses_t && has_t ) pGrid->t()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( Subdt( pPoints, ivD ) ) );
		if ( uses_Cs && has_Cs ) pGrid->Cs()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdCs( pPoints, ivD ) ) );
		if ( uses_Os && has_Os ) pGrid->Os()->SetValue( ( ( vOff + 1 ) * cuv ) + uOff, CqVMStackEntry( SubdOs( pPoints, ivD ) ) );
	}

	vOff -= 1 << ( Level - 1 );
}


//---------------------------------------------------------------------
/** Destructor
 */

CqWSurf::~CqWSurf()
{
	// Unreference the vertex storage class.
	m_pPoints->Release();
}


//---------------------------------------------------------------------
/** Create the midpoint vertices for all facets on this mesh.
 */

void CqWSurf::CreateFacePoints( TqInt& iStartIndex, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os, TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	// Create face points.
	TqInt i;
	for ( i = 0; i < cFaces(); i++ )
	{
		CqWVert* pV = new CqWVert( iStartIndex++ );
		AddVert( pV );
		pFace( i ) ->CreateSubdividePoint( this, m_pPoints, pV, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	}
}


//---------------------------------------------------------------------
/** Create the midpoint vertices for all edges on this mesh.
 */

void CqWSurf::CreateEdgePoints( TqInt& iStartIndex, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os, TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	// Create edge points.
	TqInt i;
	for ( i = 0; i < cEdges(); i++ )
	{
		CqWVert* pV = new CqWVert( iStartIndex++ );
		AddVert( pV );
		pEdge( i ) ->CreateSubdividePoint( this, m_pPoints, pV, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	}
}


//---------------------------------------------------------------------
/** Move the vertex points according to the subdivision
 */

struct SqVData
{
	CqVector3D	P;
	TqFloat	s;
	TqFloat	t;
	CqColor	Cq;
	CqColor	Os;
};


void CqWSurf::SmoothVertexPoints( TqInt oldcVerts, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
                                  TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	static CqVector3D vecT;
	static TqFloat fT;
	static CqColor colT;

	// NOTE: Not entirely happy about this method, would prefer a more efficient approach!
	// Must create this array here, to ensure we only store the old points, not the subdivided ones.
	std::vector<SqVData> aVertices;
	aVertices.resize( oldcVerts );

	CqPolygonPoints* pPoints = m_pPoints;

	// Smooth vertex points
	TqInt iE, bE, sE, i;
	for ( i = 0; i < oldcVerts; i++ )
	{
		CqWVert* pV = pVert( i );
		if ( pV->cEdges() > 0 )
		{
			// Check for crease vertex
			bE = sE = 0;
			for ( iE = 0; iE < pV->cEdges(); iE++ )
			{
				if ( pV->pEdge( iE ) ->IsValid() == TqFalse ) continue;
				if ( pV->pEdge( iE ) ->IsBoundary() ) bE++;
				if ( pV->pEdge( iE ) ->Sharpness() > 0.0f ) sE++;
			}

			// Check for smooth first (most commmon case, less likely to thrash the cache).
			if ( sE <= 1 && bE == 0 )
			{
				aVertices[ i ].P = pV->GetSmoothedScalar( vecT, &CqSubdivider::SubdP, this, pPoints );
				if ( uses_s && has_s ) aVertices[ i ].s = pV->GetSmoothedScalar( fT, &CqSubdivider::Subds, this, pPoints );
				if ( uses_t && has_t ) aVertices[ i ].t = pV->GetSmoothedScalar( fT, &CqSubdivider::Subdt, this, pPoints );
				if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = pV->GetSmoothedScalar( colT, &CqSubdivider::SubdCs, this, pPoints );
				if ( uses_Os && has_Os ) aVertices[ i ].Os = pV->GetSmoothedScalar( colT, &CqSubdivider::SubdOs, this, pPoints );
			}
			else
			{
				// Check for user set sharp edges first.
				if ( sE > 0 )
				{
					if ( sE == 2 )
					{
						aVertices[ i ].P = pV->GetCreaseScalar( vecT, &CqSubdivider::SubdP, this, pPoints );
						if ( uses_s && has_s ) aVertices[ i ].s = pV->GetCreaseScalar( fT, &CqSubdivider::Subds, this, pPoints );
						if ( uses_t && has_t ) aVertices[ i ].t = pV->GetCreaseScalar( fT, &CqSubdivider::Subdt, this, pPoints );
						if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = pV->GetCreaseScalar( colT, &CqSubdivider::SubdCs, this, pPoints );
						if ( uses_Os && has_Os ) aVertices[ i ].Os = pV->GetCreaseScalar( colT, &CqSubdivider::SubdOs, this, pPoints );
					}
					else
					{
						aVertices[ i ].P = SubdP( pPoints, pV->iVertex() );
						if ( uses_s && has_s ) aVertices[ i ].s = Subds( pPoints, pV->iVertex() );
						if ( uses_t && has_t ) aVertices[ i ].t = Subdt( pPoints, pV->iVertex() );
						if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = SubdCs( pPoints, pV->iVertex() );
						if ( uses_Os && has_Os ) aVertices[ i ].Os = SubdOs( pPoints, pV->iVertex() );
					}
				}
				else
				{
					if ( m_bInterpolateBoundary && pV->cEdges() == 2 )      	// Boundary point with valence 2 is corner
					{
						aVertices[ i ].P = SubdP( pPoints, pV->iVertex() );
						if ( uses_s && has_s ) aVertices[ i ].s = Subds( pPoints, pV->iVertex() );
						if ( uses_t && has_t ) aVertices[ i ].t = Subdt( pPoints, pV->iVertex() );
						if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = SubdCs( pPoints, pV->iVertex() );
						if ( uses_Os && has_Os ) aVertices[ i ].Os = SubdOs( pPoints, pV->iVertex() );
					}
					else				// Boundary points are crease points.
					{
						aVertices[ i ].P = pV->GetBoundaryScalar( vecT, &CqSubdivider::SubdP, this, pPoints );
						if ( uses_s && has_s ) aVertices[ i ].s = pV->GetBoundaryScalar( fT, &CqSubdivider::Subds, this, pPoints );
						if ( uses_t && has_t ) aVertices[ i ].t = pV->GetBoundaryScalar( fT, &CqSubdivider::Subdt, this, pPoints );
						if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = pV->GetBoundaryScalar( colT, &CqSubdivider::SubdCs, this, pPoints );
						if ( uses_Os && has_Os ) aVertices[ i ].Os = pV->GetBoundaryScalar( colT, &CqSubdivider::SubdOs, this, pPoints );
					}
				}
			}
		}
	}

	// Copy the modified points back to the surface.
	for ( i = 0; i < oldcVerts; i++ )
	{
		pPoints->P() [ pVert( i ) ->iVertex() ] = aVertices[ i ].P;
		if ( uses_s && has_s ) pPoints->s() [ pVert( i ) ->iVertex() ] = aVertices[ i ].s;
		if ( uses_t && has_t ) pPoints->t() [ pVert( i ) ->iVertex() ] = aVertices[ i ].t;
		if ( uses_Cs && has_Cs ) pPoints->Cs() [ pVert( i ) ->iVertex() ] = aVertices[ i ].Cq;
		if ( uses_Os && has_Os ) pPoints->Os() [ pVert( i ) ->iVertex() ] = aVertices[ i ].Os;
	}
}


//---------------------------------------------------------------------
/** Split the surface into smaller patches
 */

TqInt CqWSurf::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt cE;

	if ( !m_fSubdivided )
	{
		Subdivide();

		cE = cFaces();

		int i;
		for ( i = 0; i < cE; i++ )
		{
			CqWSurf* pNew = new CqWSurf( this, i );
			pNew->AddRef();
			pNew->SetSurfaceParameters( *m_pPoints );
			pNew->m_fDiceable = TqTrue;
			pNew->m_EyeSplitCount = m_EyeSplitCount;
			aSplits.push_back( pNew );
		}
	}
	else
	{
		cE = m_apFaces[ 0 ] ->cEdges();

		Subdivide();

		int i;
		for ( i = 0; i < cE; i++ )
		{
			CqWSurf* pNew = new CqWSurf( this, i );
			pNew->AddRef();
			pNew->SetSurfaceParameters( *m_pPoints );
			pNew->m_fDiceable = TqTrue;
			pNew->m_EyeSplitCount = m_EyeSplitCount;
			aSplits.push_back( pNew );
		}
	}

	return ( cE );
}

//---------------------------------------------------------------------
/** Transform the control hull by the specified matrix.
 */

void CqWSurf::Transform( const CqMatrix& matTx, const CqMatrix& matITTx, const CqMatrix& matRTx )
{
	// Tansform the control hull by the specified matrix.
	TqUint i;
	for ( i = 0; i < m_pPoints->P().Size(); i++ )
		m_pPoints->P() [ i ] = matTx * m_pPoints->P() [ i ];
}


//---------------------------------------------------------------------
/** Create a new C4D polygonobject from this winged edge surface.
 */

void CqWSurf::_OutputMesh( char* pname )
{
	FILE * pf = fopen( pname, "w" );
	TqInt i;
	for ( i = 0; i < cFaces(); i++ )
	{
		CqWReference ref( pFace( i ) ->pEdge( 0 ), pFace( i ) );
		CqVector3D vecA = SubdP( m_pPoints, ref.pvHead() ->iVertex() );
		ref.peNext();
		TqInt j = 1;
		while ( j < pFace( i ) ->cEdges() )
		{
			CqVector3D	vecB, vecC;
			vecB = SubdP( m_pPoints, ref.pvHead() ->iVertex() );
			vecC = SubdP( m_pPoints, ref.pvTail() ->iVertex() );

			fprintf( pf, "%f %f %f " , vecA.x(), vecA.y(), vecA.z() );
			fprintf( pf, "%f %f %f " , vecB.x(), vecB.y(), vecB.z() );
			fprintf( pf, "%f %f %f\n", vecC.x(), vecC.y(), vecC.z() );

			ref.peNext();
			j++;
		}
	}
	fclose( pf );
}


//---------------------------------------------------------------------
/** Reset this reference to the specifed edge on the specified face.
 */

void	CqWReference::Reset( CqWEdge* pE, CqWFace* pF )
{
	m_pEdge = pE;
	m_pFace = pF;

	if ( pE != 0 && pF != 0 )
	{
		if ( pE->pfRight() != pF && pE->pfLeft() != pF )
		{
			//throw("Error : Tried to create an edge reference with an invalid face/edge pair");
			return ;
		}
		m_bLeft = TqFalse;
		if ( m_pEdge->pfLeft() == m_pFace ) m_bLeft = TqTrue;
	}
}


//---------------------------------------------------------------------
/** Get the next edge of the coupled face, taking into account orientation.
 */

CqWReference& CqWReference::peNext()
{
	if ( m_bLeft ) m_pEdge = m_pEdge->peTailLeft();
	else	m_pEdge = m_pEdge->peHeadRight();

	m_bLeft = TqFalse;
	if ( m_pEdge->pfLeft() == m_pFace ) m_bLeft = TqTrue;

	return ( *this );
}


//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWReference& CqWReference::pePrev()
{
	if ( m_bLeft ) m_pEdge = m_pEdge->peHeadLeft();
	else	m_pEdge = m_pEdge->peTailRight();

	m_bLeft = TqFalse;
	if ( m_pEdge->pfLeft() == m_pFace ) m_bLeft = TqTrue;

	return ( *this );
}

//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWEdge* CqWReference::peHeadRight()
{
	if ( m_bLeft ) return ( m_pEdge->peHeadRight() );
	else	return ( m_pEdge->peTailLeft() );
}


//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWEdge* CqWReference::peHeadLeft()
{
	if ( m_bLeft ) return ( m_pEdge->peHeadLeft() );
	else	return ( m_pEdge->peTailRight() );
}


//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWEdge* CqWReference::peTailRight()
{
	if ( m_bLeft ) return ( m_pEdge->peTailRight() );
	else	return ( m_pEdge->peHeadLeft() );
}


//---------------------------------------------------------------------
/** Get the previous edge of the coupled face, taking into account orientation.
 */

CqWEdge* CqWReference::peTailLeft()
{
	if ( m_bLeft ) return ( m_pEdge->peTailLeft() );
	else	return ( m_pEdge->peHeadRight() );
}


//---------------------------------------------------------------------
/** Get the head vertex of this edge, taking into account orientation with respect to the coupled face.
 */

CqWVert* CqWReference::pvHead()
{
	if ( m_bLeft ) return ( m_pEdge->pvHead() );
	else	return ( m_pEdge->pvTail() );
}


//---------------------------------------------------------------------
/** Get the tail vertex of this edge, taking into account orientation with respect to the coupled face.
 */

CqWVert* CqWReference::pvTail()
{
	if ( m_bLeft ) return ( m_pEdge->pvTail() );
	else	return ( m_pEdge->pvHead() );
}


//---------------------------------------------------------------------
/** Get the subdivided head half edge of this edge, taking into account orientation with respect to the coupled face.
 */

CqWEdge* CqWReference::peHeadHalf()
{
	if ( m_bLeft ) return ( m_pEdge->peHeadHalf() );
	else	return ( m_pEdge->peTailHalf() );
}


//---------------------------------------------------------------------
/** Get the subdivided tail half edge of this edge, taking into account orientation with respect to the coupled face.
 */

CqWEdge* CqWReference::peTailHalf()
{
	if ( m_bLeft ) return ( m_pEdge->peTailHalf() );
	else	return ( m_pEdge->peHeadHalf() );
}


//---------------------------------------------------------------------
/** Set the left face reference for the subdivided tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpfTailLeft( CqWFace* pfTailLeft )
{
	if ( m_bLeft ) m_pEdge->peTailHalf() ->SetpfLeft( pfTailLeft );
	else	m_pEdge->peHeadHalf() ->SetpfRight( pfTailLeft );
}


//---------------------------------------------------------------------
/** Set the left face reference for the subdivided head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpfHeadLeft( CqWFace* pfHeadLeft )
{
	if ( m_bLeft ) m_pEdge->peHeadHalf() ->SetpfLeft( pfHeadLeft );
	else	m_pEdge->peTailHalf() ->SetpfRight( pfHeadLeft );
}


//---------------------------------------------------------------------
/** Set the right face reference for the subdivided tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpfTailRight( CqWFace* pfTailRight )
{
	if ( m_bLeft ) m_pEdge->peTailHalf() ->SetpfRight( pfTailRight );
	else	m_pEdge->peHeadHalf() ->SetpfLeft( pfTailRight );
}


//---------------------------------------------------------------------
/** Set the right face reference for the subdivided head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpfHeadRight( CqWFace* pfHeadRight )
{
	if ( m_bLeft ) m_pEdge->peHeadHalf() ->SetpfRight( pfHeadRight );
	else	m_pEdge->peTailHalf() ->SetpfLeft( pfHeadRight );
}


//---------------------------------------------------------------------
/** Set the winged edge tail left reference of the tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeTailTailLeft( CqWEdge* pe )
{
	if ( m_bLeft ) m_pEdge->peTailHalf() ->SetpeTailLeft( pe );
	else	m_pEdge->peHeadHalf() ->SetpeHeadRight( pe );
}


//---------------------------------------------------------------------
/** Set the winged edge head left reference of the tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeTailHeadLeft( CqWEdge* pe )
{
	if ( m_bLeft ) m_pEdge->peTailHalf() ->SetpeHeadLeft( pe );
	else	m_pEdge->peHeadHalf() ->SetpeTailRight( pe );
}


//---------------------------------------------------------------------
/** Set the winged edge tail right reference of the tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeTailTailRight( CqWEdge* pe )
{
	if ( m_bLeft ) m_pEdge->peTailHalf() ->SetpeTailRight( pe );
	else	m_pEdge->peHeadHalf() ->SetpeHeadLeft( pe );
}


//---------------------------------------------------------------------
/** Set the winged edge head right reference of the tail half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeTailHeadRight( CqWEdge* pe )
{
	if ( m_bLeft ) m_pEdge->peTailHalf() ->SetpeTailRight( pe );
	else	m_pEdge->peHeadHalf() ->SetpeHeadLeft( pe );
}


//---------------------------------------------------------------------
/** Set the winged edge tail left reference of the head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeHeadTailLeft( CqWEdge* pe )
{
	if ( m_bLeft ) m_pEdge->peHeadHalf() ->SetpeTailLeft( pe );
	else	m_pEdge->peTailHalf() ->SetpeHeadRight( pe );
}


//---------------------------------------------------------------------
/** Set the winged edge head left reference of the head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeHeadHeadLeft( CqWEdge* pe )
{
	if ( m_bLeft ) m_pEdge->peHeadHalf() ->SetpeHeadLeft( pe );
	else	m_pEdge->peTailHalf() ->SetpeTailRight( pe );
}


//---------------------------------------------------------------------
/** Set the winged edge tail right reference of the head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeHeadTailRight( CqWEdge* pe )
{
	if ( m_bLeft ) m_pEdge->peHeadHalf() ->SetpeTailRight( pe );
	else	m_pEdge->peTailHalf() ->SetpeHeadLeft( pe );
}


//---------------------------------------------------------------------
/** Set the winged edge head right reference of the head half of this edge,
 * taking into account the orientation with respect to the coupled face
 */

void CqWReference::SetpeHeadHeadRight( CqWEdge* pe )
{
	if ( m_bLeft ) m_pEdge->peHeadHalf() ->SetpeTailRight( pe );
	else	m_pEdge->peTailHalf() ->SetpeHeadLeft( pe );
}


//---------------------------------------------------------------------
/** Constructor
 */

CqWSurf::CqWSurf( CqWSurf* pSurf, TqInt iFace )
{
	m_fSubdivided = true;

	InterpolateBoundary( pSurf->bInterpolateBoundary() );

	// Allocate a new points class for points storage.
	CqPolygonPoints* pPointsClass = new CqPolygonPoints( pSurf->cVerts() );
	pPointsClass->SetSurfaceParameters( *pSurf->pPoints() );
	m_pPoints = pPointsClass;
	pPointsClass->AddRef();

	TqInt lUses = pSurf->Uses();
	TqBool uses_s = USES( lUses, EnvVars_s );
	TqBool uses_t = USES( lUses, EnvVars_t );
	TqBool uses_Cs = USES( lUses, EnvVars_Cs );
	TqBool uses_Os = USES( lUses, EnvVars_Os );

	TqBool has_s = pSurf->bHass();
	TqBool has_t = pSurf->bHast();
	TqBool has_Cs = pSurf->bHasCs();
	TqBool has_Os = pSurf->bHasOs();

	// Copy the donor face and all of its neghbours into our local face storage.
	CqWFace* pF = pSurf->pFace( iFace );
	TqInt i;
	std::vector<CqWEdge*> apEdges;

	// Initialise the P() array to a sensible size first.
	m_pPoints->P().SetSize( 0 );

	if ( pSurf->pPoints() ->Cs().Size() == 1 ) m_pPoints->Cs() = pSurf->pPoints() ->Cs();
	else	m_pPoints->Cs().SetSize( 0 );
	if ( pSurf->pPoints() ->Os().Size() == 1 ) m_pPoints->Os() = pSurf->pPoints() ->Os();
	else	m_pPoints->Os().SetSize( 0 );

	// Add the main face by adding each edge, by adding each vertex.
	CqWReference rEdge( pF->pEdge( 0 ), pF );
	apEdges.resize( pF->cEdges() );
	for ( i = 0; i < pF->cEdges(); i++ )
	{
		CqWVert* pvA = TransferVert( pSurf, rEdge.pvHead() ->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
		CqWVert* pvB = TransferVert( pSurf, rEdge.pvTail() ->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );

		apEdges[ i ] = AddEdge( pvA, pvB );
		apEdges[ i ] ->SetSharpness( rEdge.peCurrent() ->Sharpness() );

		rEdge.peNext();
	}
	// Now add the facet
	AddFace( &apEdges[ 0 ], pF->cEdges() );

	// Now do the same for each surrounding face.
	rEdge.Reset( pF->pEdge( 0 ), pF );

	for ( i = 0; i < pF->cEdges(); i++ )
	{
		CqWVert* pvTail = rEdge.pvTail();
		rEdge.peNext();
		TqInt j;
		for ( j = 0; j < pvTail->cEdges(); j++ )
		{
			CqWEdge* peCurr = pvTail->pEdge( j );
			// Only if this edge is not a part of the main facet.
			if ( peCurr->pfLeft() != pF && peCurr->pfRight() != pF )
			{
				CqWFace * pF2 = ( peCurr->pvTail() == pvTail ) ? peCurr->pfRight() : peCurr->pfLeft();
				if ( pF2 != NULL )
				{
					CqWReference rEdge2( pF2->pEdge( 0 ), pF2 );
					apEdges.resize( pF2->cEdges() );
					TqInt e;
					for ( e = 0; e < pF2->cEdges(); e++ )
					{
						CqWVert* pvA = TransferVert( pSurf, rEdge2.pvHead() ->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
						CqWVert* pvB = TransferVert( pSurf, rEdge2.pvTail() ->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );

						apEdges[ e ] = AddEdge( pvA, pvB );
						apEdges[ e ] ->SetSharpness( rEdge2.peCurrent() ->Sharpness() );

						rEdge2.peNext();
					}
					// Now add the facet
					AddFace( &apEdges[ 0 ], pF2->cEdges() );
				}
			}
		}
	}
	m_cExpectedVertices = pPoints() ->cVertex();
	m_cExpectedFaces = cFaces();
}


CqWVert* CqWSurf::TransferVert( CqWSurf* pSurf, TqInt iVert, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
                                TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	CqWVert * pNew = GetpWVert( m_pPoints, pSurf->pPoints() ->P() [ iVert ] );
	TqUint iV = pNew->iVertex();

	if ( m_pPoints->P().Size() <= iV ) m_pPoints->P().SetSize( iV + 1 );
	m_pPoints->P() [ iV ] = pSurf->pPoints() ->P() [ iVert ];

	if ( uses_s && has_s )
	{
		if ( m_pPoints->s().Size() <= iV ) m_pPoints->s().SetSize( iV + 1 );
		m_pPoints->s() [ iV ] = pSurf->pPoints() ->s() [ iVert ];
	}

	if ( uses_t && has_t )
	{
		if ( m_pPoints->t().Size() <= iV ) m_pPoints->t().SetSize( iV + 1 );
		m_pPoints->t() [ iV ] = pSurf->pPoints() ->t() [ iVert ];
	}

	if ( uses_Cs && has_Cs )
	{
		if ( m_pPoints->Cs().Size() <= iV ) m_pPoints->Cs().SetSize( iV + 1 );
		m_pPoints->Cs() [ iV ] = pSurf->pPoints() ->Cs() [ iVert ];
	}

	if ( uses_Os && has_Os )
	{
		if ( m_pPoints->Os().Size() <= iV ) m_pPoints->Os().SetSize( iV + 1 );
		m_pPoints->Os() [ iV ] = pSurf->pPoints() ->Os() [ iVert ];
	}

	return ( pNew );
}


//---------------------------------------------------------------------
/** Add a vertex to the list.
 */

CqWVert* CqWSurf::GetpWVert( CqPolygonPoints* pPoints, const CqVector3D& V )
{
	CqWVert * pExist = FindVertex( pPoints, V );
	if ( pExist != 0 )
		return ( pExist );
	else
	{
		TqInt iV = pPoints->P().Size();
		CqWVert* pNew = new CqWVert( iV );
		m_apVerts.push_back( pNew );
		return ( pNew );
	}
}


//---------------------------------------------------------------------
/** Determine whether the patch can be diced based on its screen size, if so work
 * out how many subdivisions to perform to get a MP grid and store it.
 */

CqBound CqWSurf::Bound() const
{
	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqUint i;
	for ( i = 0; i < m_pPoints->P().Size(); i++ )
	{
		CqVector3D	vecV = m_pPoints->P() [ i ];
		if ( vecV.x() < vecA.x() ) vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() ) vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() ) vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() ) vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() ) vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() ) vecB.z( vecV.z() );
	}
	CqBound	B;
	B.vecMin() = vecA;
	B.vecMax() = vecB;
	return ( B );
}


//---------------------------------------------------------------------
/** Determine whether the patch can be diced based on its screen size, if so work
 * out how many subdivisions to perform to get a MP grid and store it.
 */

TqBool CqWSurf::Diceable()
{
	// Fail if not a quad patch
	TqInt iF;
	for ( iF = 0; iF < cFaces(); iF++ )
		if ( pFace( iF ) ->cEdges() != 4 ) return ( TqFalse );

	const CqMatrix& matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	// Get the sides of the main quad (the first if everything goes according to plan.
	CqVector3D vecA = m_pPoints->P() [ pFace( 0 ) ->pEdge( 0 ) ->pvHead() ->iVertex() ];
	CqVector3D vecB = m_pPoints->P() [ pFace( 0 ) ->pEdge( 1 ) ->pvHead() ->iVertex() ];
	CqVector3D vecC = m_pPoints->P() [ pFace( 0 ) ->pEdge( 2 ) ->pvHead() ->iVertex() ];
	CqVector3D vecD = m_pPoints->P() [ pFace( 0 ) ->pEdge( 3 ) ->pvHead() ->iVertex() ];

	vecA = matCtoR * vecA;
	vecB = matCtoR * vecB;
	vecC = matCtoR * vecC;
	vecD = matCtoR * vecD;

	TqFloat lA = ( vecB - vecA ).Magnitude2();
	TqFloat lB = ( vecC - vecB ).Magnitude2();
	TqFloat lC = ( vecD - vecC ).Magnitude2();
	TqFloat lD = ( vecA - vecD ).Magnitude2();

	TqFloat l = MAX( lA, MAX( lB, ( MAX( lC, lD ) ) ) );

	l = sqrt( l );
	//	l=ROUND(l);

	// Get the shading rate.
	float ShadingRate = pAttributes() ->fEffectiveShadingRate();
	l /= ShadingRate;

	if ( l > 16 ) return ( TqFalse );
	else
	{
		m_DiceCount = static_cast<TqInt>( l );
		m_DiceCount = CEIL_POW2( m_DiceCount );
		m_DiceCount = ( m_DiceCount == 16 ) ? 4 : ( m_DiceCount == 8 ) ? 3 : ( m_DiceCount == 4 ) ? 2 : ( m_DiceCount == 2 ) ? 1 : 0;
		return ( TqTrue );
	}
}


//---------------------------------------------------------------------
/** Dice the patch into a micropolygon grid for shading and rendering.
 */

CqMicroPolyGridBase* CqWSurf::Dice()
{
	// Create a new CqMicroPolyGrid for this patch
	TqInt cuv = ( 1 << m_DiceCount );
	CqMicroPolyGrid* pGrid = new CqMicroPolyGrid( cuv, cuv, m_pPoints );

	TqInt lUses = Uses();
	TqBool uses_s = USES( lUses, EnvVars_s );
	TqBool uses_t = USES( lUses, EnvVars_t );
	TqBool uses_Cs = USES( lUses, EnvVars_Cs );
	TqBool uses_Os = USES( lUses, EnvVars_Os );

	TqBool has_s = bHass();
	TqBool has_t = bHast();
	TqBool has_Cs = bHasCs();
	TqBool has_Os = bHasOs();

	if ( uses_Cs && !has_Cs ) m_pPoints->Cs().BilinearDice( cuv, cuv, pGrid->Cs() );
	if ( uses_Os && !has_Os ) m_pPoints->Os().BilinearDice( cuv, cuv, pGrid->Os() );

	DiceSubdivide( m_DiceCount );

	TqInt iFace = 0;
	StoreDice( m_DiceCount, iFace, m_pPoints, 0, 0, cuv + 1, pGrid, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	return ( pGrid );
}


//---------------------------------------------------------------------
/** Get the bound of this GPrim.
 */

CqBound	CqMotionWSurf::Bound() const
{
	CqBound B( FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		TqUint j;
		for ( j = 0; j < GetMotionObject( Time( i ) ) ->P().Size(); j++ )
		{
			CqVector3D	vecV = GetMotionObject( Time( i ) ) ->P() [ j ];
			B.Encapsulate( vecV );
		}
	}
	return ( B );
}


//---------------------------------------------------------------------
/** Split the surface into smaller patches
 */

TqInt CqMotionWSurf::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt cE;

	if ( !m_fSubdivided )
	{
		Subdivide();

		cE = cFaces();

		int i;
		for ( i = 0; i < cE; i++ )
		{
			CqMotionWSurf* pNew = new CqMotionWSurf( this, i );
			pNew->AddRef();
			pNew->SetSurfaceParameters( *GetMotionObject( Time( 0 ) ) );
			pNew->m_fDiceable = TqTrue;
			pNew->m_EyeSplitCount = m_EyeSplitCount;
			aSplits.push_back( pNew );
		}
	}
	else
	{
		cE = m_apFaces[ 0 ] ->cEdges();

		Subdivide();

		int i;
		for ( i = 0; i < cE; i++ )
		{
			CqMotionWSurf* pNew = new CqMotionWSurf( this, i );
			pNew->AddRef();
			pNew->SetSurfaceParameters( *GetMotionObject( Time( 0 ) ) );
			pNew->m_fDiceable = TqTrue;
			pNew->m_EyeSplitCount = m_EyeSplitCount;
			aSplits.push_back( pNew );
		}
	}

	return ( cE );
}


//---------------------------------------------------------------------
/** Determine whether the patch can be diced based on its screen size, if so work
 * out how many subdivisions to perform to get a MP grid and store it.
 */

TqBool CqMotionWSurf::Diceable()
{
	// Fail if not a quad patch
	TqInt iF;
	for ( iF = 0; iF < cFaces(); iF++ )
		if ( pFace( iF ) ->cEdges() != 4 ) return ( TqFalse );

	const CqMatrix& matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	// Get the sides of the main quad (the first if everything goes according to plan.
	CqPolygonPoints* pPoints = GetMotionObject( Time( 0 ) );
	CqVector3D vecA = pPoints->P() [ pFace( 0 ) ->pEdge( 0 ) ->pvHead() ->iVertex() ];
	CqVector3D vecB = pPoints->P() [ pFace( 0 ) ->pEdge( 1 ) ->pvHead() ->iVertex() ];
	CqVector3D vecC = pPoints->P() [ pFace( 0 ) ->pEdge( 2 ) ->pvHead() ->iVertex() ];
	CqVector3D vecD = pPoints->P() [ pFace( 0 ) ->pEdge( 3 ) ->pvHead() ->iVertex() ];

	vecA = matCtoR * vecA;
	vecB = matCtoR * vecB;
	vecC = matCtoR * vecC;
	vecD = matCtoR * vecD;

	TqFloat lA = ( vecB - vecA ).Magnitude2();
	TqFloat lB = ( vecC - vecB ).Magnitude2();
	TqFloat lC = ( vecD - vecC ).Magnitude2();
	TqFloat lD = ( vecA - vecD ).Magnitude2();

	TqFloat l = MAX( lA, MAX( lB, ( MAX( lC, lD ) ) ) );

	l = sqrt( l );
	//	l=ROUND(l);


	// Get the shading rate.
	float ShadingRate = pAttributes() ->fEffectiveShadingRate();
	l /= ShadingRate;

	if ( l > 16 ) return ( TqFalse );
	else
	{
		m_DiceCount = static_cast<TqInt>( l );
		m_DiceCount = CEIL_POW2( m_DiceCount );
		m_DiceCount = ( m_DiceCount == 16 ) ? 4 : ( m_DiceCount == 8 ) ? 3 : ( m_DiceCount == 4 ) ? 2 : ( m_DiceCount == 2 ) ? 1 : 0;
		return ( TqTrue );
	}
}


//---------------------------------------------------------------------
/** Dice the patch into a micropolygon grid for shading and rendering.
 */

CqMicroPolyGridBase* CqMotionWSurf::Dice()
{
	// Create a new CqMicroPolyGrid for this patch
	TqInt cuv = ( 1 << m_DiceCount );

	TqInt lUses = Uses();
	TqBool uses_s = USES( lUses, EnvVars_s );
	TqBool uses_t = USES( lUses, EnvVars_t );
	TqBool uses_Cs = USES( lUses, EnvVars_Cs );
	TqBool uses_Os = USES( lUses, EnvVars_Os );

	TqBool has_s = bHass();
	TqBool has_t = bHast();
	TqBool has_Cs = bHasCs();
	TqBool has_Os = bHasOs();

	// Dice Subdivide at all time slots.
	DiceSubdivide( m_DiceCount );

	CqMotionMicroPolyGrid* pGrid = new CqMotionMicroPolyGrid;
	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		CqPolygonPoints* pPoints = GetMotionObject( Time( i ) );
		CqMicroPolyGrid* pGrid2 = new CqMicroPolyGrid( cuv, cuv, pPoints );

		if ( uses_Cs && !has_Cs ) pPoints->Cs().BilinearDice( cuv, cuv, pGrid2->Cs() );
		if ( uses_Os && !has_Os ) pPoints->Os().BilinearDice( cuv, cuv, pGrid2->Os() );

		TqInt iFace = 0;
		StoreDice( m_DiceCount, iFace, pPoints, 0, 0, cuv + 1, pGrid2, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
		pGrid->AddTimeSlot( Time( i ), pGrid2 );
	}
	return ( pGrid );
}


//---------------------------------------------------------------------
/** Create the midpoint vertices for all facets on this mesh.
 */

void CqMotionWSurf::CreateFacePoints( TqInt& iStartIndex, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os, TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	// Create face points.
	TqInt i;
	for ( i = 0; i < cFaces(); i++ )
	{
		CqWVert* pV = new CqWVert( iStartIndex++ );
		AddVert( pV );

		TqInt j;
		for ( j = 0; j < cTimes(); j++ )
			pFace( i ) ->CreateSubdividePoint( this, GetMotionObject( Time( j ) ), pV, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	}
}


//---------------------------------------------------------------------
/** Create the midpoint vertices for all edges on this mesh.
 */

void CqMotionWSurf::CreateEdgePoints( TqInt& iStartIndex, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os, TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	// Create edge points.
	TqInt i;
	for ( i = 0; i < cEdges(); i++ )
	{
		CqWVert* pV = new CqWVert( iStartIndex++ );
		AddVert( pV );
		TqInt j;
		for ( j = 0; j < cTimes(); j++ )
			pEdge( i ) ->CreateSubdividePoint( this, GetMotionObject( Time( j ) ), pV, uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
	}
}


void CqMotionWSurf::SmoothVertexPoints( TqInt oldcVerts, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
                                        TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	static CqVector3D vecT;
	static TqFloat fT;
	static CqColor colT;

	// NOTE: Not entirely happy about this method, would prefer a more efficient approach!
	// Must create this array here, to ensure we only store the old points, not the subdivided ones.
	std::vector<SqVData> aVertices;
	aVertices.resize( oldcVerts );

	TqInt iTime;

	for ( iTime = 0; iTime < cTimes(); iTime++ )
	{
		CqPolygonPoints* pPoints = GetMotionObject( Time( iTime ) );

		// Smooth vertex points
		TqInt iE, bE, sE, i;
		for ( i = 0; i < oldcVerts; i++ )
		{
			CqWVert* pV = pVert( i );
			if ( pV->cEdges() > 0 )
			{
				// Check for crease vertex
				bE = sE = 0;
				for ( iE = 0; iE < pV->cEdges(); iE++ )
				{
					if ( pV->pEdge( iE ) ->IsValid() == TqFalse ) continue;
					if ( pV->pEdge( iE ) ->IsBoundary() ) bE++;
					if ( pV->pEdge( iE ) ->Sharpness() > 0.0f ) sE++;
				}

				// Check for smooth first (most commmon case, less likely to thrash the cache).
				if ( sE <= 1 && bE == 0 )
				{
					aVertices[ i ].P = pV->GetSmoothedScalar( vecT, &CqSubdivider::SubdP, this, pPoints );
					if ( uses_s && has_s ) aVertices[ i ].s = pV->GetSmoothedScalar( fT, &CqSubdivider::Subds, this, pPoints );
					if ( uses_t && has_t ) aVertices[ i ].t = pV->GetSmoothedScalar( fT, &CqSubdivider::Subdt, this, pPoints );
					if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = pV->GetSmoothedScalar( colT, &CqSubdivider::SubdCs, this, pPoints );
					if ( uses_Os && has_Os ) aVertices[ i ].Os = pV->GetSmoothedScalar( colT, &CqSubdivider::SubdOs, this, pPoints );
				}
				else
				{
					// Check for user set sharp edges first.
					if ( sE > 0 )
					{
						if ( sE == 2 )
						{
							aVertices[ i ].P = pV->GetCreaseScalar( vecT, &CqSubdivider::SubdP, this, pPoints );
							if ( uses_s && has_s ) aVertices[ i ].s = pV->GetCreaseScalar( fT, &CqSubdivider::Subds, this, pPoints );
							if ( uses_t && has_t ) aVertices[ i ].t = pV->GetCreaseScalar( fT, &CqSubdivider::Subdt, this, pPoints );
							if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = pV->GetCreaseScalar( colT, &CqSubdivider::SubdCs, this, pPoints );
							if ( uses_Os && has_Os ) aVertices[ i ].Os = pV->GetCreaseScalar( colT, &CqSubdivider::SubdOs, this, pPoints );
						}
						else
						{
							aVertices[ i ].P = SubdP( pPoints, pV->iVertex() );
							if ( uses_s && has_s ) aVertices[ i ].s = Subds( pPoints, pV->iVertex() );
							if ( uses_t && has_t ) aVertices[ i ].t = Subdt( pPoints, pV->iVertex() );
							if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = SubdCs( pPoints, pV->iVertex() );
							if ( uses_Os && has_Os ) aVertices[ i ].Os = SubdOs( pPoints, pV->iVertex() );
						}
					}
					else
					{
						if ( pV->cEdges() == 2 )      	// Boundary point with valence 2 is corner
						{
							aVertices[ i ].P = SubdP( pPoints, pV->iVertex() );
							if ( uses_s && has_s ) aVertices[ i ].s = Subds( pPoints, pV->iVertex() );
							if ( uses_t && has_t ) aVertices[ i ].t = Subdt( pPoints, pV->iVertex() );
							if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = SubdCs( pPoints, pV->iVertex() );
							if ( uses_Os && has_Os ) aVertices[ i ].Os = SubdOs( pPoints, pV->iVertex() );
						}
						else				// Boundary points are crease points.
						{
							aVertices[ i ].P = pV->GetBoundaryScalar( vecT, &CqSubdivider::SubdP, this, pPoints );
							if ( uses_s && has_s ) aVertices[ i ].s = pV->GetBoundaryScalar( fT, &CqSubdivider::Subds, this, pPoints );
							if ( uses_t && has_t ) aVertices[ i ].t = pV->GetBoundaryScalar( fT, &CqSubdivider::Subdt, this, pPoints );
							if ( uses_Cs && has_Cs ) aVertices[ i ].Cq = pV->GetBoundaryScalar( colT, &CqSubdivider::SubdCs, this, pPoints );
							if ( uses_Os && has_Os ) aVertices[ i ].Os = pV->GetBoundaryScalar( colT, &CqSubdivider::SubdOs, this, pPoints );
						}
					}
				}
			}
		}

		// Copy the modified points back to the surface.
		for ( i = 0; i < oldcVerts; i++ )
		{
			pPoints->P() [ pVert( i ) ->iVertex() ] = aVertices[ i ].P;
			if ( uses_s && has_s ) pPoints->s() [ pVert( i ) ->iVertex() ] = aVertices[ i ].s;
			if ( uses_t && has_t ) pPoints->t() [ pVert( i ) ->iVertex() ] = aVertices[ i ].t;
			if ( uses_Cs && has_Cs ) pPoints->Cs() [ pVert( i ) ->iVertex() ] = aVertices[ i ].Cq;
			if ( uses_Os && has_Os ) pPoints->Os() [ pVert( i ) ->iVertex() ] = aVertices[ i ].Os;
		}
	}
}


//---------------------------------------------------------------------
/** Constructor
 */

CqMotionWSurf::CqMotionWSurf( CqMotionWSurf* pSurf, TqInt iFace ) : CqMotionSpec<CqPolygonPoints*>( 0 )
{
	m_fSubdivided = true;

	// Allocate a new points class for points storage.
	TqInt i;
	for ( i = 0; i < pSurf->cTimes(); i++ )
	{
		CqPolygonPoints* pPointsClass = new CqPolygonPoints( pSurf->cVerts() );
		pPointsClass->SetSurfaceParameters( *pSurf->GetMotionObject( Time( i ) ) );
		pPointsClass->AddRef();
		AddTimeSlot( pSurf->Time( i ), pPointsClass );

		CqPolygonPoints* pSurfPoints = pSurf->GetMotionObject( pSurf->Time( i ) );

		// Initialise the P() array to a sensible size first.
		pPointsClass->P().SetSize( 0 );
		if ( pSurfPoints->Cs().Size() == 1 ) pPointsClass->Cs() = pSurfPoints->Cs();
		else	pPointsClass->Cs().SetSize( 0 );
		if ( pSurfPoints->Os().Size() == 1 ) pPointsClass->Os() = pSurfPoints->Os();
		else	pPointsClass->Os().SetSize( 0 );
	}

	TqInt lUses = pSurf->Uses();
	TqBool uses_s = USES( lUses, EnvVars_s );
	TqBool uses_t = USES( lUses, EnvVars_t );
	TqBool uses_Cs = USES( lUses, EnvVars_Cs );
	TqBool uses_Os = USES( lUses, EnvVars_Os );

	TqBool has_s = pSurf->bHass();
	TqBool has_t = pSurf->bHast();
	TqBool has_Cs = pSurf->bHasCs();
	TqBool has_Os = pSurf->bHasOs();

	// Copy the donor face and all of its neghbours into our local face storage.
	CqWFace* pF = pSurf->pFace( iFace );
	std::vector<CqWEdge*> apEdges;

	// Add the main face by adding each edge, by adding each vertex.
	CqWReference rEdge( pF->pEdge( 0 ), pF );
	apEdges.resize( pF->cEdges() );
	for ( i = 0; i < pF->cEdges(); i++ )
	{
		CqWVert* pvA = TransferVert( pSurf, rEdge.pvHead() ->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
		CqWVert* pvB = TransferVert( pSurf, rEdge.pvTail() ->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );

		apEdges[ i ] = AddEdge( pvA, pvB );
		apEdges[ i ] ->SetSharpness( rEdge.peCurrent() ->Sharpness() );

		rEdge.peNext();
	}
	// Now add the facet
	AddFace( &apEdges[ 0 ], pF->cEdges() );

	// Now do the same for each surrounding face.
	rEdge.Reset( pF->pEdge( 0 ), pF );

	for ( i = 0; i < pF->cEdges(); i++ )
	{
		CqWVert* pvTail = rEdge.pvTail();
		rEdge.peNext();
		TqInt j;
		for ( j = 0; j < pvTail->cEdges(); j++ )
		{
			CqWEdge* peCurr = pvTail->pEdge( j );
			// Only if this edge is not a part of the main facet.
			if ( peCurr->pfLeft() != pF && peCurr->pfRight() != pF )
			{
				CqWFace * pF2 = ( peCurr->pvTail() == pvTail ) ? peCurr->pfRight() : peCurr->pfLeft();
				if ( pF2 != NULL )
				{
					CqWReference rEdge2( pF2->pEdge( 0 ), pF2 );
					apEdges.resize( pF2->cEdges() );
					TqInt e;
					for ( e = 0; e < pF2->cEdges(); e++ )
					{
						CqWVert* pvA = TransferVert( pSurf, rEdge2.pvHead() ->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );
						CqWVert* pvB = TransferVert( pSurf, rEdge2.pvTail() ->iVertex(), uses_s, uses_t, uses_Cs, uses_Os, has_s, has_t, has_Cs, has_Os );

						apEdges[ e ] = AddEdge( pvA, pvB );
						apEdges[ e ] ->SetSharpness( rEdge2.peCurrent() ->Sharpness() );

						rEdge2.peNext();
					}
					// Now add the facet
					AddFace( &apEdges[ 0 ], pF2->cEdges() );
				}
			}
		}
	}
	m_cExpectedVertices = GetMotionObject( Time( 0 ) ) ->cVertex();
	m_cExpectedFaces = cFaces();
}


CqWVert* CqMotionWSurf::TransferVert( CqMotionWSurf* pSurf, TqInt iVert, TqBool uses_s, TqBool uses_t, TqBool uses_Cs, TqBool uses_Os,
                                      TqBool has_s, TqBool has_t, TqBool has_Cs, TqBool has_Os )
{
	// Check if the point exists, at time 0, if so it should be available at all times.
	CqWVert * pNew = GetpWVert( GetMotionObject( Time( 0 ) ), pSurf->GetMotionObject( Time( 0 ) ) ->P() [ iVert ] );
	TqUint iV = pNew->iVertex();

	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		CqPolygonPoints* pMyPoints = GetMotionObject( i );
		CqPolygonPoints* pSurfPoints = pSurf->GetMotionObject( i );

		if ( pMyPoints->P().Size() <= iV ) pMyPoints->P().SetSize( iV + 1 );
		pMyPoints->P() [ iV ] = pSurfPoints->P() [ iVert ];

		if ( uses_s && has_s )
		{
			if ( pMyPoints->s().Size() <= iV ) pMyPoints->s().SetSize( iV + 1 );
			pMyPoints->s() [ iV ] = pSurfPoints->s() [ iVert ];
		}

		if ( uses_t && has_t )
		{
			if ( pMyPoints->t().Size() <= iV ) pMyPoints->t().SetSize( iV + 1 );
			pMyPoints->t() [ iV ] = pSurfPoints->t() [ iVert ];
		}

		if ( uses_Cs && has_Cs )
		{
			if ( pMyPoints->Cs().Size() <= iV ) pMyPoints->Cs().SetSize( iV + 1 );
			pMyPoints->Cs() [ iV ] = pSurfPoints->Cs() [ iVert ];
		}

		if ( uses_Os && has_Os )
		{
			if ( pMyPoints->Os().Size() <= iV ) pMyPoints->Os().SetSize( iV + 1 );
			pMyPoints->Os() [ iV ] = pSurfPoints->Os() [ iVert ];
		}
	}

	return ( pNew );
}


//---------------------------------------------------------------------
/** Add a vertex to the list.
 */

CqWVert* CqMotionWSurf::GetpWVert( CqPolygonPoints* pPoints, const CqVector3D& V )
{
	CqWVert * pExist = FindVertex( pPoints, V );
	if ( pExist != 0 )
		return ( pExist );
	else
	{
		TqInt iV = pPoints->P().Size();
		CqWVert* pNew = new CqWVert( iV );
		m_apVerts.push_back( pNew );
		return ( pNew );
	}
}


END_NAMESPACE( Aqsis )
