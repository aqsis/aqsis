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

DEFINE_STATIC_MEMORYPOOL( CqWEdge, 512 );
DEFINE_STATIC_MEMORYPOOL( CqWFace, 512 );
DEFINE_STATIC_MEMORYPOOL( CqWVert, 512 );

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

CqWVert* CqWEdge::CreateSubdividePoint( CqSubdivider* pSurf, CqPolygonPoints* pPoints, CqWVert* pV )
{
	TqUint index = pV->iVertex();
	m_pvSubdivide = pV;

	std::vector<CqParameter*>::iterator iUP;
	for( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++ )
	{
		// We only need to apply subdivision rules to 'vertex' class variables...
		if( (*iUP)->Class() == class_vertex )
			CreateSubdivideScalar( (*iUP), (*iUP), index );
		else
			CreateSubdivideScalar( (*iUP), (*iUP), index, TqTrue );
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

CqWVert* CqWFace::CreateSubdividePoint( CqSubdivider* pSurf, CqPolygonPoints* pPoints, CqWVert* pV )
{
	TqUint index = pV->iVertex();
	m_pvSubdivide = pV;

	std::vector<CqParameter*>::iterator iUP;
	for( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++ )
		CreateSubdivideScalar( (*iUP), (*iUP), index );

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

CqWVert* CqSubdivider::FindVertex( CqPolygonPoints* pPoints, const CqVector4D& V )
{
	// If no vertices or no edges, cannot have been constructed yet.
	if ( m_apVerts.size() == 0 || m_apEdges.size() == 0 ) return ( NULL );

	for ( std::vector<CqWVert*>::iterator i = m_apVerts.begin(); i != m_apVerts.end(); i++ )
	{
		if ( (*pPoints->P()) [ ( *i ) ->iVertex() ] == V )
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
	CreateFacePoints( index );
	// Create edge points.
	CreateEdgePoints( index );
	// Smooth vertex points
	SmoothVertexPoints( oldcVerts );

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
	CreateFacePoints( index );
	// Create edge points.
	CreateEdgePoints( index );
	// Smooth vertex points
	SmoothVertexPoints( oldcVerts );


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



void StoreDiceAPVar( IqShader* pShader, CqParameter* pParam, TqUint ivA, TqUint indexA )
{
	// Find the argument
	IqShaderData* pArg = pShader->FindArgument( pParam->strName() );
	if( NULL != pArg )
	{
		switch( pParam->Type() )
		{
			case type_float:
			{
				CqParameterTyped<TqFloat, TqFloat>* pNParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>(pParam);
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_integer:
			{
				CqParameterTyped<TqInt, TqFloat>* pNParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>(pParam);
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_point:
			case type_vector:
			case type_normal:
			{
				CqParameterTyped<CqVector3D, CqVector3D>* pNParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>(pParam);
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_hpoint:
			{
				CqParameterTyped<CqVector4D, CqVector3D>* pNParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>(pParam);
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_string:
			{
				CqParameterTyped<CqString, CqString>* pNParam = static_cast<CqParameterTyped<CqString, CqString>*>(pParam);
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_color:
			{
				CqParameterTyped<CqColor, CqColor>* pNParam = static_cast<CqParameterTyped<CqColor, CqColor>*>(pParam);
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;

			case type_matrix:
			{
				CqParameterTyped<CqMatrix, CqMatrix>* pNParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>(pParam);
				pArg->SetValue( *pNParam->pValue( ivA ), indexA );
			}
			break;
		}
	}
}


void CqSubdivider::StoreDice( TqInt Level, TqInt& iFace, CqPolygonPoints* pPoints,
                              TqInt uOff, TqInt vOff, TqInt cuv, CqMicroPolyGrid* pGrid )
{
	CqWFace * pF;
	CqWReference rE;

	TqUint indexA = ( ( vOff ) * cuv ) + uOff;
	TqUint indexB = ( ( vOff ) * cuv ) + uOff + 1;
	TqUint indexC = ( ( vOff + 1 ) * cuv ) + uOff + 1;
	TqUint indexD = ( ( vOff + 1 ) * cuv ) + uOff;

	TqInt lUses = Uses();

	if ( Level > 1 )
		StoreDice( Level - 1, iFace, pPoints, uOff, vOff, cuv, pGrid );
	else
	{
		pF = pFace( iFace++ );
		rE.Reset( pF->pEdge( 0 ), pF );
		TqInt ivA = rE.pvHead() ->iVertex();
		TqInt ivB = rE.peNext().pvHead() ->iVertex();
		TqInt ivC = rE.peNext().pvHead() ->iVertex();
		TqInt ivD = rE.peNext().pvHead() ->iVertex();

		if( USES( Uses(), EnvVars_P ) )
		{
			pGrid->P()->SetPoint( (*pPoints->P())[ ivA ], indexA );
			pGrid->P()->SetPoint( (*pPoints->P())[ ivB ], indexB );
			pGrid->P()->SetPoint( (*pPoints->P())[ ivC ], indexC );
			pGrid->P()->SetPoint( (*pPoints->P())[ ivD ], indexD );
		}

		if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->s() ) && bHass() ) 
		{
			pGrid->s()->SetFloat( (*pPoints->s())[ ivA ], indexA );
			pGrid->s()->SetFloat( (*pPoints->s())[ ivB ], indexB );
			pGrid->s()->SetFloat( (*pPoints->s())[ ivC ], indexC );
			pGrid->s()->SetFloat( (*pPoints->s())[ ivD ], indexD );
		}

		if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->t() ) && bHast() ) 
		{
			pGrid->t()->SetFloat( (*pPoints->t())[ ivA ], indexA );
			pGrid->t()->SetFloat( (*pPoints->t())[ ivB ], indexB );
			pGrid->t()->SetFloat( (*pPoints->t())[ ivC ], indexC );
			pGrid->t()->SetFloat( (*pPoints->t())[ ivD ], indexD );
		}

		if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->u() ) && bHasu() ) 
		{
			pGrid->u()->SetFloat( (*pPoints->u())[ ivA ], indexA );
			pGrid->u()->SetFloat( (*pPoints->u())[ ivB ], indexB );
			pGrid->u()->SetFloat( (*pPoints->u())[ ivC ], indexC );
			pGrid->u()->SetFloat( (*pPoints->u())[ ivD ], indexD );
		}

		if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->v() ) && bHasv() ) 
		{
			pGrid->v()->SetFloat( (*pPoints->v())[ ivA ], indexA );
			pGrid->v()->SetFloat( (*pPoints->v())[ ivB ], indexB );
			pGrid->v()->SetFloat( (*pPoints->v())[ ivC ], indexC );
			pGrid->v()->SetFloat( (*pPoints->v())[ ivD ], indexD );
		}


		// Now lets store the diced user specified primitive variables.
		std::vector<CqParameter*>::iterator iUP;
		for( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++ )
		{
			/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
			if( NULL != pGrid->pAttributes()->pshadSurface() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivA, indexA );
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivB, indexB );
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivC, indexC );
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivD, indexD );
			}

			if( NULL != pGrid->pAttributes()->pshadDisplacement() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivA, indexA );
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivB, indexB );
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivC, indexC );
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivD, indexD );
			}

			if( NULL != pGrid->pAttributes()->pshadAtmosphere() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivA, indexA );
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivB, indexB );
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivC, indexC );
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivD, indexD );
			}
		}	
	}

	uOff += 1 << ( Level - 1 );
	indexB = ( ( vOff ) * cuv ) + uOff + 1;
	indexC = ( ( vOff + 1 ) * cuv ) + uOff + 1;
	if ( Level > 1 )
		StoreDice( Level - 1, iFace, pPoints, uOff, vOff, cuv, pGrid );
	else
	{
		pF = pFace( iFace++ );
		rE.Reset( pF->pEdge( 1 ), pF );
		TqInt ivB = rE.pvHead() ->iVertex();
		TqInt ivC = rE.peNext().pvHead() ->iVertex();

		if( USES( Uses(), EnvVars_P ) )
		{
			pGrid->P()->SetPoint( (*pPoints->P())[ ivB ], indexB );
			pGrid->P()->SetPoint( (*pPoints->P())[ ivC ], indexC );
		}

		if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->s() ) && bHass() ) 
		{
			pGrid->s()->SetFloat( (*pPoints->s())[ ivB ], indexB );
			pGrid->s()->SetFloat( (*pPoints->s())[ ivC ], indexC );
		}

		if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->t() ) && bHast() ) 
		{
			pGrid->t()->SetFloat( (*pPoints->t())[ ivB ], indexB );
			pGrid->t()->SetFloat( (*pPoints->t())[ ivC ], indexC );
		}

		if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->u() ) && bHasu() ) 
		{
			pGrid->u()->SetFloat( (*pPoints->u())[ ivB ], indexB );
			pGrid->u()->SetFloat( (*pPoints->u())[ ivC ], indexC );
		}

		if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->v() ) && bHasv() ) 
		{
			pGrid->v()->SetFloat( (*pPoints->v())[ ivB ], indexB );
			pGrid->v()->SetFloat( (*pPoints->v())[ ivC ], indexC );
		}

		// Now lets store the diced user specified primitive variables.
		std::vector<CqParameter*>::iterator iUP;
		for( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++ )
		{
			/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
			if( NULL != pGrid->pAttributes()->pshadSurface() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivB, indexB );
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivC, indexC );
			}

			if( NULL != pGrid->pAttributes()->pshadDisplacement() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivB, indexB );
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivC, indexC );
			}

			if( NULL != pGrid->pAttributes()->pshadAtmosphere() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivB, indexB );
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivC, indexC );
			}
		}	
	}

	vOff += 1 << ( Level - 1 );
	indexC = ( ( vOff + 1 ) * cuv ) + uOff + 1;
	indexD = ( ( vOff + 1 ) * cuv ) + uOff;
	if ( Level > 1 )
		StoreDice( Level - 1, iFace, pPoints, uOff, vOff, cuv, pGrid );
	else
	{
		pF = pFace( iFace++ );
		rE.Reset( pF->pEdge( 2 ), pF );
		TqInt ivC = rE.pvHead() ->iVertex();
		TqInt ivD = rE.peNext().pvHead() ->iVertex();

		if( USES( Uses(), EnvVars_P ) )
		{
			pGrid->P()->SetPoint( (*pPoints->P())[ ivC ], indexC );
			pGrid->P()->SetPoint( (*pPoints->P())[ ivD ], indexD );
		}

		if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->s() ) && bHass() ) 
		{
			pGrid->s()->SetFloat( (*pPoints->s())[ ivC ], indexC );
			pGrid->s()->SetFloat( (*pPoints->s())[ ivD ], indexD );
		}

		if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->t() ) && bHast() ) 
		{
			pGrid->t()->SetFloat( (*pPoints->t())[ ivC ], indexC );
			pGrid->t()->SetFloat( (*pPoints->t())[ ivD ], indexD );
		}

		if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->u() ) && bHasu() ) 
		{
			pGrid->u()->SetFloat( (*pPoints->u())[ ivC ], indexC );
			pGrid->u()->SetFloat( (*pPoints->u())[ ivD ], indexD );
		}

		if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->v() ) && bHasv() ) 
		{
			pGrid->v()->SetFloat( (*pPoints->v())[ ivC ], indexC );
			pGrid->v()->SetFloat( (*pPoints->v())[ ivD ], indexD );
		}

		// Now lets store the diced user specified primitive variables.
		std::vector<CqParameter*>::iterator iUP;
		for( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++ )
		{
			/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
			if( NULL != pGrid->pAttributes()->pshadSurface() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivC, indexC );
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivD, indexD );
			}

			if( NULL != pGrid->pAttributes()->pshadDisplacement() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivC, indexC );
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivD, indexD );
			}

			if( NULL != pGrid->pAttributes()->pshadAtmosphere() )
			{
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivC, indexC );
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivD, indexD );
			}
		}	
	}

	uOff -= 1 << ( Level - 1 );
	indexD = ( ( vOff + 1 ) * cuv ) + uOff;
	if ( Level > 1 )
		StoreDice( Level - 1, iFace, pPoints, uOff, vOff, cuv, pGrid );
	else
	{
		pF = pFace( iFace++ );
		rE.Reset( pF->pEdge( 3 ), pF );
		TqInt ivD = rE.pvHead() ->iVertex();

		if( USES( Uses(), EnvVars_P ) )
			pGrid->P()->SetPoint( (*pPoints->P())[ ivD ], indexD );

		if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->s() ) && bHass() ) 
			pGrid->s()->SetFloat( (*pPoints->s())[ ivD ], indexD );

		if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->t() ) && bHast() ) 
			pGrid->t()->SetFloat( (*pPoints->t())[ ivD ], indexD );

		if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->u() ) && bHasu() ) 
			pGrid->u()->SetFloat( (*pPoints->u())[ ivD ], indexD );

		if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->v() ) && bHasv() ) 
			pGrid->v()->SetFloat( (*pPoints->v())[ ivD ], indexD );

		// Now lets store the diced user specified primitive variables.
		std::vector<CqParameter*>::iterator iUP;
		for( iUP = pPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++ )
		{
			/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
			if( NULL != pGrid->pAttributes()->pshadSurface() )
				StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), (*iUP), ivD, indexD );

			if( NULL != pGrid->pAttributes()->pshadDisplacement() )
				StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), (*iUP), ivD, indexD );

			if( NULL != pGrid->pAttributes()->pshadAtmosphere() )
				StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), (*iUP), ivD, indexD );
		}	
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

void CqWSurf::CreateFacePoints( TqInt& iStartIndex )
{
	// Create face points.
	TqInt i;
	for ( i = 0; i < cFaces(); i++ )
	{
		CqWVert* pV = new CqWVert( iStartIndex++ );
		AddVert( pV );
		pFace( i ) ->CreateSubdividePoint( this, m_pPoints, pV );
	}
}


//---------------------------------------------------------------------
/** Create the midpoint vertices for all edges on this mesh.
 */

void CqWSurf::CreateEdgePoints( TqInt& iStartIndex )
{
	// Create edge points.
	TqInt i;
	for ( i = 0; i < cEdges(); i++ )
	{
		CqWVert* pV = new CqWVert( iStartIndex++ );
		AddVert( pV );
		pEdge( i ) ->CreateSubdividePoint( this, m_pPoints, pV );
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


#ifdef AQSIS_SYSTEM_MACOSX
// Workaround for Mac OS X gcc 2.95 compiler error -
// "Fixup [linenumber] too large for field width of 16 bits"
#pragma CC_OPT_OFF
#endif

void CqWSurf::SmoothVertexPoints( TqInt oldcVerts )
{
	static CqVector3D vecT;
	static TqFloat fT;
	static CqColor colT;

	// NOTE: Not entirely happy about this method, would prefer a more efficient approach!
	// Must create this array here, to ensure we only store the old points, not the subdivided ones.
	CqPolygonPoints* pPoints = m_pPoints;
	CqPolygonPoints* pNewPoints = new CqPolygonPoints(*pPoints);
	pNewPoints->ClonePrimitiveVariables(*pPoints);

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
				std::vector<CqParameter*>::iterator iUP, iNUP;
				for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
					if( (*iUP)->Class() == class_vertex )
						pV->GetSmoothedScalar( (*iUP), (*iNUP), i );
			}
			else
			{
				// Check for user set sharp edges first.
				if ( sE > 0 )
				{
					if ( sE == 2 )
					{
						std::vector<CqParameter*>::iterator iUP, iNUP;
						for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
							if( (*iUP)->Class() == class_vertex )
								pV->GetCreaseScalar( (*iUP), (*iNUP), i );
					}
					else
					{
						std::vector<CqParameter*>::iterator iUP, iNUP;
						for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
							if( (*iUP)->Class() == class_vertex )
								pV->GetCornerScalar( (*iUP), (*iNUP), i );
					}
				}
				else
				{
					if ( m_bInterpolateBoundary && pV->cEdges() == 2 )      	// Boundary point with valence 2 is corner
					{
						std::vector<CqParameter*>::iterator iUP, iNUP;
						for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
							if( (*iUP)->Class() == class_vertex )
								pV->GetCornerScalar( (*iUP), (*iNUP), i );
					}
					else				// Boundary points are crease points.
					{
						std::vector<CqParameter*>::iterator iUP, iNUP;
						for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
							if( (*iUP)->Class() == class_vertex )
								pV->GetBoundaryScalar( (*iUP), (*iNUP), i );
					}
				}
			}
		}
	}

	// Copy the modified points back to the surface.
	m_pPoints = pNewPoints;
	m_pPoints->AddRef();
	pPoints->Release();
}

#ifdef AQSIS_SYSTEM_MACOSX
#pragma CC_OPT_RESTORE
#endif

//---------------------------------------------------------------------
/** Split the surface into smaller patches
 */

TqInt CqWSurf::Split( std::vector<CqBasicSurface*>& aSplits )
{
	TqInt cE;

	if ( !m_fSubdivided )
	{
		//Subdivide();
		cE = cFaces();
		m_fSubdivided = TqTrue;

		int i;
		for ( i = 0; i < cE; i++ )
		{
			CqBasicSurface* pNew = ExtractFace( i );
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
		m_fSubdivided = TqTrue;

		int i;
		for ( i = 0; i < cE; i++ )
		{
			CqBasicSurface* pNew = ExtractFace( i );
			pNew->AddRef();
			pNew->SetSurfaceParameters( *m_pPoints );
			pNew->m_fDiceable = TqTrue;
			pNew->m_EyeSplitCount = m_EyeSplitCount;
			aSplits.push_back( pNew );
		}
	}

	return ( cE );
}


CqBasicSurface* CqWSurf::ExtractFace( TqInt index)
{
	CqWFace* pThisFace = pFace( index );

//	if( pThisFace->cEdges() == 4 &&
//		pThisFace->pEdge(0)->pvHead()->cEdges() == 4 &&
//		pThisFace->pEdge(1)->pvHead()->cEdges() == 4 &&
//		pThisFace->pEdge(2)->pvHead()->cEdges() == 4 &&
//		pThisFace->pEdge(3)->pvHead()->cEdges() == 4 )
//	{
		// This is a pure quad based face, so just extract it as a b-spline mesh
//	}
//	else
	{
		CqWSurf* pNew = new CqWSurf(this, index);

		TqInt MyUses = Uses();

		// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
		if( USES( MyUses, EnvVars_u ) && !bHasu() )
		{
			pNew->pPoints()->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("u") );
			pNew->pPoints()->u()->SetSize(4);
			pNew->pPoints()->u()->pValue( 0 )[0] = 0.0f;
			pNew->pPoints()->u()->pValue( 1 )[0] = 1.0f;
			pNew->pPoints()->u()->pValue( 2 )[0] = 0.0f;
			pNew->pPoints()->u()->pValue( 3 )[0] = 1.0f;
		}

		// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
		if( USES( MyUses, EnvVars_v ) && !bHasv() )
		{
			pNew->pPoints()->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
			pNew->pPoints()->v()->SetSize(4);
			pNew->pPoints()->v()->pValue( 0 )[0] = 0.0f;
			pNew->pPoints()->v()->pValue( 1 )[0] = 0.0f;
			pNew->pPoints()->v()->pValue( 2 )[0] = 1.0f;
			pNew->pPoints()->v()->pValue( 3 )[0] = 1.0f;
		}

		return( pNew );
	}
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
		CqVector3D vecA = (*m_pPoints->P())[ ref.pvHead() ->iVertex() ];
		ref.peNext();
		TqInt j = 1;
		while ( j < pFace( i ) ->cEdges() )
		{
			CqVector3D	vecB, vecC;
			vecB = (*m_pPoints->P())[ ref.pvHead() ->iVertex() ];
			vecC = (*m_pPoints->P())[ ref.pvTail() ->iVertex() ];

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
	m_fSubdivided = pSurf->m_fSubdivided;

	InterpolateBoundary( pSurf->bInterpolateBoundary() );

	// Allocate a new points class for points storage.
	CqPolygonPoints* pPointsClass = new CqPolygonPoints( pSurf->cVerts(), 1 );
	pPointsClass->SetSurfaceParameters( *pSurf->pPoints() );
	m_pPoints = pPointsClass;
	pPointsClass->AddRef();

	TqInt lUses = pSurf->Uses();

	// Copy the donor face and all of its neghbours into our local face storage.
	CqWFace* pF = pSurf->pFace( iFace );
	TqInt i;
	std::vector<CqWEdge*> apEdges;

	// If we have any APV's then make sure that we copy them.
	std::vector<CqParameter*>::iterator iUP;
	for( iUP = pSurf->pPoints()->aUserParams().begin(); iUP != pSurf->pPoints()->aUserParams().end(); iUP++ )
	{
		CqParameter* pNewPV = (*iUP)->CloneType( (*iUP)->strName().c_str(), (*iUP)->Count() );
		pNewPV->Clear();
		m_pPoints->AddPrimitiveVariable( pNewPV );
	}

	// Add the main face by adding each edge, by adding each vertex.
	CqWReference rEdge( pF->pEdge( 0 ), pF );
	apEdges.resize( pF->cEdges() );
	for ( i = 0; i < pF->cEdges(); i++ )
	{
		CqWVert* pvA = TransferVert( pSurf, rEdge.pvHead() ->iVertex() );
		CqWVert* pvB = TransferVert( pSurf, rEdge.pvTail() ->iVertex() );

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
						CqWVert* pvA = TransferVert( pSurf, rEdge2.pvHead() ->iVertex() );
						CqWVert* pvB = TransferVert( pSurf, rEdge2.pvTail() ->iVertex() );

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


//---------------------------------------------------------------------
/** Transfer vertex information between surfaces.
 */

CqWVert* CqWSurf::TransferVert( CqWSurf* pSurf, TqInt iVert )
{
	CqWVert * pNew = GetpWVert( m_pPoints, (*pSurf->pPoints() ->P()) [ iVert ] );
	TqUint iV = pNew->iVertex();

	std::vector<CqParameter*>::iterator iUP, iTUP;
	for( iUP = pSurf->pPoints()->aUserParams().begin(), iTUP = m_pPoints->aUserParams().begin(); iUP != pSurf->pPoints()->aUserParams().end(); iUP++, iTUP++ )
	{
		if ( (*iTUP)->Size() <= iV ) (*iTUP)->SetSize( iV + 1 );
		CqParameterTypedVarying<TqFloat, type_float, TqFloat>* pTT = static_cast<CqParameterTypedVarying<TqFloat, type_float, TqFloat>*>((*iUP));
		(*iTUP)->SetValue( (*iUP), iV, iVert );
	}

	return ( pNew );
}


//---------------------------------------------------------------------
/** Add a vertex to the list.
 */

CqWVert* CqWSurf::GetpWVert( CqPolygonPoints* pPoints, const CqVector4D& V )
{
	CqWVert * pExist = FindVertex( pPoints, V );
	if ( pExist != 0 )
		return ( pExist );
	else
	{
		TqInt iV = pPoints->P()->Size();
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
	for ( i = 0; i < m_pPoints->P()->Size(); i++ )
	{
		CqVector3D	vecV = (*m_pPoints->P()) [ i ];
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

#ifdef AQSIS_SYSTEM_MACOSX
// Workaround for Mac OS X gcc 2.95 compiler error -
// "Fixup [linenumber] too large for field width of 16 bits"
#pragma CC_OPT_OFF
#endif

TqBool CqWSurf::Diceable()
{
	assert( NULL != m_pPoints->P() );

	if( !m_fSubdivided )	return( TqFalse );
	
	// Fail if not a quad patch
	TqInt iF;
	for ( iF = 0; iF < cFaces(); iF++ )
		if ( pFace( iF ) ->cEdges() != 4 ) return ( TqFalse );

	const CqMatrix& matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	// Get the sides of the main quad (the first if everything goes according to plan.
	CqVector3D vecA = (*m_pPoints->P()) [ pFace( 0 ) ->pEdge( 0 ) ->pvHead() ->iVertex() ];
	CqVector3D vecB = (*m_pPoints->P()) [ pFace( 0 ) ->pEdge( 1 ) ->pvHead() ->iVertex() ];
	CqVector3D vecC = (*m_pPoints->P()) [ pFace( 0 ) ->pEdge( 2 ) ->pvHead() ->iVertex() ];
	CqVector3D vecD = (*m_pPoints->P()) [ pFace( 0 ) ->pEdge( 3 ) ->pvHead() ->iVertex() ];

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
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute("System", "ShadingRate")[0];
	l /= ShadingRate;

	if ( l > 16 ) return ( TqFalse );
	else
	{
		m_DiceCount = static_cast<TqInt>( l );
		m_DiceCount = CEIL_POW2( m_DiceCount );
		m_DiceCount = ( m_DiceCount == 16 ) ? 4 : ( m_DiceCount == 8 ) ? 3 : ( m_DiceCount == 4 ) ? 2 : ( m_DiceCount == 2 ) ? 1 : 1;
		return ( TqTrue );
	}
}

#ifdef AQSIS_SYSTEM_MACOSX
#pragma CC_OPT_RESTORE
#endif

//---------------------------------------------------------------------
/** Dice the patch into a micropolygon grid for shading and rendering.
 */

CqMicroPolyGridBase* CqWSurf::Dice()
{
	// Create a new CqMicroPolyGrid for this patch
	TqInt cuv = ( 1 << m_DiceCount );
	CqMicroPolyGrid* pGrid = new CqMicroPolyGrid( cuv, cuv, m_pPoints );

	TqInt lUses = Uses();

	// Dice the primitive variables.

	DiceSubdivide( m_DiceCount );

	TqInt iFace = 0;
	StoreDice( m_DiceCount, iFace, m_pPoints, 0, 0, cuv + 1, pGrid );

	// If the color and opacity are not defined, use the system values.
	if ( USES( lUses, EnvVars_Cs ) && !bHasCs() ) 
	{
		if( NULL != pAttributes()->GetColorAttribute("System", "Color") )
			pGrid->Cs()->SetColor( pAttributes()->GetColorAttribute("System", "Color")[0]);
		else
			pGrid->Cs()->SetColor( CqColor( 1,1,1 ) );
	}

	if ( USES( lUses, EnvVars_Os ) && !bHasOs() ) 
	{
		if( NULL != pAttributes()->GetColorAttribute("System", "Opacity") )
			pGrid->Os()->SetColor( pAttributes()->GetColorAttribute("System", "Opacity")[0]);
		else
			pGrid->Os()->SetColor( CqColor( 1,1,1 ) );
	}

	// Now we need to dice the user specified parameters as appropriate.
//	std::vector<CqParameter*>::iterator iUP;
//	for( iUP = m_pPoints->aUserParams().begin(); iUP != m_pPoints->aUserParams().end(); iUP++ )
//	{
//		/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
//		if( NULL != pGrid->pAttributes()->pshadSurface() )
//			pGrid->pAttributes()->pshadSurface()->SetArgument( (*iUP), this );
//
//		if( NULL != pGrid->pAttributes()->pshadDisplacement() )
//			pGrid->pAttributes()->pshadDisplacement()->SetArgument( (*iUP), this );
//
//		if( NULL != pGrid->pAttributes()->pshadAtmosphere() )
//			pGrid->pAttributes()->pshadAtmosphere()->SetArgument( (*iUP), this );
//	}

	return ( pGrid );
}


//---------------------------------------------------------------------
/** Destructor
 */

CqMotionWSurf::~CqMotionWSurf()
{
	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		CqPolygonPoints* pPts;
		if( NULL != ( pPts = GetMotionObject( Time( i ) ) ) )
			pPts->Release();
	}
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
		for ( j = 0; j < GetMotionObject( Time( i ) ) ->P()->Size(); j++ )
		{
			CqVector3D	vecV = (*GetMotionObject( Time( i ) ) ->P()) [ j ];
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
		//Subdivide();
		cE = cFaces();
		m_fSubdivided = TqTrue;

		int i;
		for ( i = 0; i < cE; i++ )
		{
			CqBasicSurface* pNew = ExtractFace( i );
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
			CqBasicSurface* pNew = ExtractFace( i );
			pNew->AddRef();
			pNew->SetSurfaceParameters( *GetMotionObject( Time( 0 ) ) );
			pNew->m_fDiceable = TqTrue;
			pNew->m_EyeSplitCount = m_EyeSplitCount;
			aSplits.push_back( pNew );
		}
	}

	return ( cE );
}


CqBasicSurface* CqMotionWSurf::ExtractFace( TqInt index)
{
	CqWFace* pThisFace = pFace( index );

//	if( pThisFace->cEdges() == 4 &&
//		pThisFace->pEdge(0)->pvHead()->cEdges() == 4 &&
//		pThisFace->pEdge(1)->pvHead()->cEdges() == 4 &&
//		pThisFace->pEdge(2)->pvHead()->cEdges() == 4 &&
//		pThisFace->pEdge(3)->pvHead()->cEdges() == 4 )
//	{
		// This is a pure quad based face, so just extract it as a b-spline mesh
//	}
//	else
	{
		CqMotionWSurf* pNew = new CqMotionWSurf(this, index);

		TqInt MyUses = Uses();

		TqInt i;
		for( i = 0; i < pNew->cTimes(); i++ )
		{
			CqPolygonPoints* pPoints = pNew->GetMotionObject( pNew->Time( i ) );

			// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
			if( USES( MyUses, EnvVars_u ) && !bHasu() )
			{
				pPoints->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("u") );
				pPoints->u()->SetSize(4);
				pPoints->u()->pValue( 0 )[0] = 0.0f;
				pPoints->u()->pValue( 1 )[0] = 1.0f;
				pPoints->u()->pValue( 2 )[0] = 0.0f;
				pPoints->u()->pValue( 3 )[0] = 1.0f;
			}

			// If the shaders need u/v or s/t and they are not specified, then we need to put them in as defaults.
			if( USES( MyUses, EnvVars_v ) && !bHasv() )
			{
				pPoints->AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>("v") );
				pPoints->v()->SetSize(4);
				pPoints->v()->pValue( 0 )[0] = 0.0f;
				pPoints->v()->pValue( 1 )[0] = 0.0f;
				pPoints->v()->pValue( 2 )[0] = 1.0f;
				pPoints->v()->pValue( 3 )[0] = 1.0f;
			}
		}

		return( pNew );
	}
}


//---------------------------------------------------------------------
/** Determine whether the patch can be diced based on its screen size, if so work
 * out how many subdivisions to perform to get a MP grid and store it.
 */

#ifdef AQSIS_SYSTEM_MACOSX
// Workaround for Mac OS X gcc 2.95 compiler error -
// "Fixup [linenumber] too large for field width of 16 bits"
#pragma CC_OPT_OFF
#endif

TqBool CqMotionWSurf::Diceable()
{
	if( !m_fSubdivided )	return( TqFalse );
	
	// Fail if not a quad patch
	TqInt iF;
	for ( iF = 0; iF < cFaces(); iF++ )
		if ( pFace( iF ) ->cEdges() != 4 ) return ( TqFalse );

	const CqMatrix& matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	// Get the sides of the main quad (the first if everything goes according to plan.
	CqPolygonPoints* pPoints = GetMotionObject( Time( 0 ) );
	CqVector3D vecA = (*pPoints->P()) [ pFace( 0 ) ->pEdge( 0 ) ->pvHead() ->iVertex() ];
	CqVector3D vecB = (*pPoints->P()) [ pFace( 0 ) ->pEdge( 1 ) ->pvHead() ->iVertex() ];
	CqVector3D vecC = (*pPoints->P()) [ pFace( 0 ) ->pEdge( 2 ) ->pvHead() ->iVertex() ];
	CqVector3D vecD = (*pPoints->P()) [ pFace( 0 ) ->pEdge( 3 ) ->pvHead() ->iVertex() ];

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
	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute("System", "ShadingRate")[0];
	l /= ShadingRate;

	if ( l > 16 ) return ( TqFalse );
	else
	{
		m_DiceCount = static_cast<TqInt>( l );
		m_DiceCount = CEIL_POW2( m_DiceCount );
		m_DiceCount = ( m_DiceCount == 16 ) ? 4 : ( m_DiceCount == 8 ) ? 3 : ( m_DiceCount == 4 ) ? 2 : ( m_DiceCount == 2 ) ? 1 : 1;
		return ( TqTrue );
	}
}

#ifdef AQSIS_SYSTEM_MACOSX
#pragma CC_OPT_RESTORE
#endif

//---------------------------------------------------------------------
/** Dice the patch into a micropolygon grid for shading and rendering.
 */

CqMicroPolyGridBase* CqMotionWSurf::Dice()
{
	// Create a new CqMicroPolyGrid for this patch
	TqInt cuv = ( 1 << m_DiceCount );

	TqInt lUses = Uses();

	// Dice Subdivide at all time slots.
	DiceSubdivide( m_DiceCount );

	CqMotionMicroPolyGrid* pGrid = new CqMotionMicroPolyGrid;
	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		CqPolygonPoints* pPoints = GetMotionObject( Time( i ) );
		CqMicroPolyGrid* pGrid2 = new CqMicroPolyGrid( cuv, cuv, pPoints );

		// If the color and opacity are not defined, use the system values.
		if ( USES( lUses, EnvVars_Cs ) && !bHasCs() ) 
		{
			if( NULL != pAttributes()->GetColorAttribute("System", "Color") )
				pGrid2->Cs()->SetColor( pAttributes()->GetColorAttribute("System", "Color")[0]);
			else
				pGrid2->Cs()->SetColor( CqColor( 1,1,1 ) );
		}

		if ( USES( lUses, EnvVars_Os ) && !bHasOs() ) 
		{
			if( NULL != pAttributes()->GetColorAttribute("System", "Opacity") )
				pGrid2->Os()->SetColor( pAttributes()->GetColorAttribute("System", "Opacity")[0]);
			else
				pGrid2->Os()->SetColor( CqColor( 1,1,1 ) );
		}

		TqInt iFace = 0;
		StoreDice( m_DiceCount, iFace, pPoints, 0, 0, cuv + 1, pGrid2 );
		pGrid->AddTimeSlot( Time( i ), pGrid2 );
	}
	return ( pGrid );
}


//---------------------------------------------------------------------
/** Create the midpoint vertices for all facets on this mesh.
 */

void CqMotionWSurf::CreateFacePoints( TqInt& iStartIndex )
{
	// Create face points.
	TqInt i;
	for ( i = 0; i < cFaces(); i++ )
	{
		CqWVert* pV = new CqWVert( iStartIndex++ );
		AddVert( pV );

		TqInt j;
		for ( j = 0; j < cTimes(); j++ )
			pFace( i ) ->CreateSubdividePoint( this, GetMotionObject( Time( j ) ), pV );
	}
}


//---------------------------------------------------------------------
/** Create the midpoint vertices for all edges on this mesh.
 */

void CqMotionWSurf::CreateEdgePoints( TqInt& iStartIndex )
{
	// Create edge points.
	TqInt i;
	for ( i = 0; i < cEdges(); i++ )
	{
		CqWVert* pV = new CqWVert( iStartIndex++ );
		AddVert( pV );
		TqInt j;
		for ( j = 0; j < cTimes(); j++ )
			pEdge( i ) ->CreateSubdividePoint( this, GetMotionObject( Time( j ) ), pV );
	}
}


void CqMotionWSurf::SmoothVertexPoints( TqInt oldcVerts )
{
	static CqVector3D vecT;
	static TqFloat fT;
	static CqColor colT;

	TqInt iTime;

	for ( iTime = 0; iTime < cTimes(); iTime++ )
	{
		CqPolygonPoints* pPoints = GetMotionObject( Time( iTime ) );
		CqPolygonPoints* pNewPoints = new CqPolygonPoints(*pPoints);
		pNewPoints->ClonePrimitiveVariables(*pPoints);

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
					std::vector<CqParameter*>::iterator iUP, iNUP;
					for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
						pV->GetSmoothedScalar( (*iUP), (*iNUP), i );
				}
				else
				{
					// Check for user set sharp edges first.
					if ( sE > 0 )
					{
						if ( sE == 2 )
						{
							std::vector<CqParameter*>::iterator iUP, iNUP;
							for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
								pV->GetCreaseScalar( (*iUP), (*iNUP), i );
						}
						else
						{
							std::vector<CqParameter*>::iterator iUP, iNUP;
							for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
								pV->GetCornerScalar( (*iUP), (*iNUP), i );
						}
					}
					else
					{
						if ( pV->cEdges() == 2 )      	// Boundary point with valence 2 is corner
						{
							std::vector<CqParameter*>::iterator iUP, iNUP;
							for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
								pV->GetCornerScalar( (*iUP), (*iNUP), i );
						}
						else				// Boundary points are crease points.
						{
							std::vector<CqParameter*>::iterator iUP, iNUP;
							for( iUP = pPoints->aUserParams().begin(), iNUP = pNewPoints->aUserParams().begin(); iUP != pPoints->aUserParams().end(); iUP++, iNUP++ )
								pV->GetBoundaryScalar( (*iUP), (*iNUP), i );
						}
					}
				}
			}
		}

		// Copy the modified points back to the surface.
		AddTimeSlot( Time( iTime ), pNewPoints );
		pNewPoints->AddRef();
		pPoints->Release();
	}
}


//---------------------------------------------------------------------
/** Constructor
 */

CqMotionWSurf::CqMotionWSurf( CqMotionWSurf* pSurf, TqInt iFace ) : CqMotionSpec<CqPolygonPoints*>( 0 )
{
	m_fSubdivided = pSurf->m_fSubdivided;

	InterpolateBoundary( pSurf->bInterpolateBoundary() );

	TqInt lUses = pSurf->Uses();

	// Allocate a new points class for points storage.
	TqInt i;
	for ( i = 0; i < pSurf->cTimes(); i++ )
	{
		CqPolygonPoints* pPointsClass = new CqPolygonPoints( pSurf->cVerts(), 1 );
		pPointsClass->SetSurfaceParameters( *pSurf->GetMotionObject( Time( i ) ) );
		pPointsClass->AddRef();
		AddTimeSlot( pSurf->Time( i ), pPointsClass );

		CqPolygonPoints* pSurfPoints = pSurf->GetMotionObject( pSurf->Time( i ) );

		// Clone any primitive variables.
		std::vector<CqParameter*>::const_iterator iUP;
		for( iUP = pSurfPoints->aUserParams().begin(); iUP != pSurfPoints->aUserParams().end(); iUP++ )
		{
			CqParameter* pNewPV = (*iUP)->CloneType( (*iUP)->strName().c_str(), (*iUP)->Count() );
			pNewPV->Clear();
			pPointsClass->AddPrimitiveVariable( pNewPV );
		}
	}


	// Copy the donor face and all of its neghbours into our local face storage.
	CqWFace* pF = pSurf->pFace( iFace );
	std::vector<CqWEdge*> apEdges;

	// Add the main face by adding each edge, by adding each vertex.
	CqWReference rEdge( pF->pEdge( 0 ), pF );
	apEdges.resize( pF->cEdges() );
	for ( i = 0; i < pF->cEdges(); i++ )
	{
		CqWVert* pvA = TransferVert( pSurf, rEdge.pvHead() ->iVertex() );
		CqWVert* pvB = TransferVert( pSurf, rEdge.pvTail() ->iVertex() );

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
						CqWVert* pvA = TransferVert( pSurf, rEdge2.pvHead() ->iVertex() );
						CqWVert* pvB = TransferVert( pSurf, rEdge2.pvTail() ->iVertex() );

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


CqWVert* CqMotionWSurf::TransferVert( CqMotionWSurf* pSurf, TqInt iVert )
{
	// Check if the point exists, at time 0, if so it should be available at all times.
	CqWVert * pNew = GetpWVert( GetMotionObject( Time( 0 ) ), (*pSurf->GetMotionObject( Time( 0 ) ) ->P()) [ iVert ] );
	TqUint iV = pNew->iVertex();

	TqInt i;
	for ( i = 0; i < cTimes(); i++ )
	{
		CqPolygonPoints* pMyPoints = GetMotionObject( i );
		CqPolygonPoints* pSurfPoints = pSurf->GetMotionObject( i );

		std::vector<CqParameter*>::iterator iUP, iTUP;
		for( iUP = pSurfPoints->aUserParams().begin(), iTUP = pMyPoints->aUserParams().begin(); iUP != pSurfPoints->aUserParams().end(); iUP++, iTUP++ )
		{
			if ( (*iTUP)->Size() <= iV ) (*iTUP)->SetSize( iV + 1 );
			(*iTUP)->SetValue( (*iUP), iV, iVert );
		}
	}

	return ( pNew );
}


//---------------------------------------------------------------------
/** Add a vertex to the list.
 */

CqWVert* CqMotionWSurf::GetpWVert( CqPolygonPoints* pPoints, const CqVector4D& V )
{
	CqWVert * pExist = FindVertex( pPoints, V );
	if ( pExist != 0 )
		return ( pExist );
	else
	{
		TqInt iV = pPoints->P()->Size();
		CqWVert* pNew = new CqWVert( iV );
		m_apVerts.push_back( pNew );
		return ( pNew );
	}
}


/** Perform the subdivision arithmetic on a paramter type.
 * \param t Temp of the template type to overcome the VC++ problem with template functions.
 * \param F A pointer to a function to get indexed values of the appropriate type.
 * \param pSurf Pointer to the CqWSurf on which we are working. 
 */
void CqWVert::GetSmoothedScalar( CqParameter* pCurrent, CqParameter* pTarget, TqUint trgIndex )
{
	// NOTE: Checks should have been made prior to this call to ensure it is neither
	// a boundary point or a crease/corner point with sharp edges.

	switch( pCurrent->Type() )
	{
		case type_float:
		{
			_SmoothScalar_(TqFloat, TqFloat);
		}
		break;

		case type_integer:
		{
			_SmoothScalar_(TqInt, TqFloat);
		}
		break;

		case type_point:
		case type_normal:
		case type_vector:
		{
			_SmoothScalar_(CqVector3D, CqVector3D);
		}
		break;

		case type_color:
		{
			_SmoothScalar_(CqColor, CqColor);
		}
		break;

		case type_hpoint:
		{
			_SmoothScalar_(CqVector4D, CqVector3D);
		}
		break;

//				case type_string:
//				{
//					_SmoothScalar_(CqString, CqString);
//				}
//				break;

//				case type_matrix:
//				{
//					_SmoothScalar_(CqMatrix, CqMatrix);
//				}
		break;
	}
}

/** Templatised function to perform the subdivision arithmetic on a paramter type.
 * \param t Temp of the template type to overcome the VC++ problem with template functions.
 * \param F A pointer to a function to get indexed values of the appropriate type.
 * \param pSurf Pointer to the CqWSurf on which we are working. 
 */
void CqWVert::GetCreaseScalar( CqParameter* pCurrent, CqParameter* pTarget, TqUint trgIndex )
{
	switch( pCurrent->Type() )
	{
		case type_float:
		{
			_CreaseScalar_(TqFloat, TqFloat);
		}
		break;

		case type_integer:
		{
			_CreaseScalar_(TqInt, TqFloat);
		}
		break;

		case type_point:
		case type_normal:
		case type_vector:
		{
			_CreaseScalar_(CqVector3D, CqVector3D);
		}
		break;

		case type_color:
		{
			_CreaseScalar_(CqColor, CqColor);
		}
		break;

		case type_hpoint:
		{
			_CreaseScalar_(CqVector4D, CqVector3D);
		}
		break;

//				case type_string:
//				{
//					_CreaseScalar_(CqString, CqString);
//				}
//				break;

//				case type_matrix:
//				{
//					_CreaseScalar_(CqMatrix, CqMatrix);
//				}
		break;
	}
}

/** Templatised function to perform the subdivision arithmetic on a paramter type.
 * \param t Temp of the template type to overcome the VC++ problem with template functions.
 * \param F A pointer to a function to get indexed values of the appropriate type.
 * \param pSurf Pointer to the CqWSurf on which we are working. 
 */
void CqWVert::GetBoundaryScalar( CqParameter* pCurrent, CqParameter* pTarget, TqUint trgIndex )
{
	switch( pCurrent->Type() )
	{
		case type_float:
		{
			_BoundaryScalar_(TqFloat, TqFloat);
		}
		break;

		case type_integer:
		{
			_BoundaryScalar_(TqInt, TqFloat);
		}
		break;

		case type_point:
		case type_normal:
		case type_vector:
		{
			_BoundaryScalar_(CqVector3D, CqVector3D);
		}
		break;

		case type_color:
		{
			_BoundaryScalar_(CqColor, CqColor);
		}
		break;

		case type_hpoint:
		{
			_BoundaryScalar_(CqVector4D, CqVector3D);
		}
		break;

//				case type_string:
//				{
//					_BoundaryScalar_(CqString, CqString);
//				}
//				break;

//				case type_matrix:
//				{
//					_BoundaryScalar_(CqMatrix, CqMatrix);
//				}
		break;
	}
}


/** Templatised function to perform the subdivision arithmetic on a paramter type.
 * \param t Temp of the template type to overcome the VC++ problem with template functions.
 * \param F A pointer to a function to get indexed values of the appropriate type.
 * \param pSurf Pointer to the CqWSurf on which we are working. 
 */
void CqWVert::GetCornerScalar( CqParameter* pCurrent, CqParameter* pTarget, TqUint trgIndex )
{
	switch( pCurrent->Type() )
	{
		case type_float:
		{
			_BoundaryScalar_(TqFloat, TqFloat);
		}
		break;

		case type_integer:
		{
			_BoundaryScalar_(TqInt, TqFloat);
		}
		break;

		case type_point:
		case type_normal:
		case type_vector:
		{
			_BoundaryScalar_(CqVector3D, CqVector3D);
		}
		break;

		case type_color:
		{
			_BoundaryScalar_(CqColor, CqColor);
		}
		break;

		case type_hpoint:
		{
			_BoundaryScalar_(CqVector4D, CqVector3D);
		}
		break;

//				case type_string:
//				{
//					_BoundaryScalar_(CqString, CqString);
//				}
//				break;

//				case type_matrix:
//				{
//					_BoundaryScalar_(CqMatrix, CqMatrix);
//				}
		break;
	}
}


/** Perform the subdivision arithmetic on a paramter type.
 * \param t Temp of the template type to overcome the VC++ problem with template functions.
 * \param F A pointer to a function to get indexed values of the appropriate type.
 * \param pSurf Pointer to the CqWSurf on which we are working. 
 */
void CqWFace::CreateSubdivideScalar( CqParameter* pCurrent, CqParameter* pTarget, TqUint trgIndex )
{
	CqWReference grE( m_apEdges[ 0 ], this );

	switch( pCurrent->Type() )
	{
		case type_float:
		{
			_SubdivideParameterFace_(TqFloat, TqFloat);
		}
		break;

		case type_integer:
		{
			_SubdivideParameterFace_(TqInt, TqFloat);
		}
		break;

		case type_point:
		case type_normal:
		case type_vector:
		{
			_SubdivideParameterFace_(CqVector3D, CqVector3D);
		}
		break;

		case type_color:
		{
			_SubdivideParameterFace_(CqColor, CqColor);
		}
		break;

		case type_hpoint:
		{
			_SubdivideParameterFace_(CqVector4D, CqVector3D);
		}
		break;

//				case type_string:
//				{
//					_SubdivideParameterFace_(CqString, CqString);
//				}
//				break;

//				case type_matrix:
//				{
//					_SubdivideParameterFace_(CqMatrix, CqMatrix);
//				}
		break;
	}
}




/** Perform the subdivision arithmetic on a paramter type.
 * \param t Temp of the template type to overcome the VC++ problem with template functions.
 * \param F A pointer to a function to get indexed values of the appropriate type.
 * \param pSurf Pointer to the CqWSurf on which we are working. 
 */
void CqWEdge::CreateSubdivideScalar( CqParameter* pCurrent, CqParameter* pTarget, TqUint trgIndex, TqBool bForceMidpoint )
{
	switch( pCurrent->Type() )
	{
		case type_float:
		{
			_SubdivideParameterEdge_(TqFloat, TqFloat);
		}
		break;

		case type_integer:
		{
			_SubdivideParameterEdge_(TqInt, TqFloat);
		}
		break;

		case type_point:
		case type_normal:
		case type_vector:
		{
			_SubdivideParameterEdge_(CqVector3D, CqVector3D);
		}
		break;

		case type_color:
		{
			_SubdivideParameterEdge_(CqColor, CqColor);
		}
		break;

		case type_hpoint:
		{
			_SubdivideParameterEdge_(CqVector4D, CqVector3D);
		}
		break;

//				case type_string:
//				{
//					_SubdivideParameterEdge_(CqString, CqString);
//				}
//				break;

//				case type_matrix:
//				{
//					_SubdivideParameterEdge_(CqMatrix, CqMatrix);
//				}
//				break;
	}
}

END_NAMESPACE( Aqsis )
