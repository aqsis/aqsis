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
		\brief Implements the classes for subdivision surfaces.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"subdivision2.h"
#include	"patch.h"

#include	<fstream>
#include	<vector>

START_NAMESPACE( Aqsis )

//------------------------------------------------------------------------------
/**
 *	Constructor.
 */

CqSubdivision2::CqSubdivision2( CqPolygonPoints* pPoints ) : m_fFinalised(TqFalse)
{
	assert(NULL != pPoints);
	// Store the reference to our points.
	m_pPoints = pPoints;
	m_pPoints->AddRef();
}


//------------------------------------------------------------------------------
/**
 *	Destructor.
 */

CqSubdivision2::~CqSubdivision2()
{
	assert(NULL != m_pPoints);
	// Delete the array of laths generated during the facet adding phase.
	for(std::vector<CqLath*>::const_iterator iLath=apLaths().begin(); iLath!=apLaths().end(); iLath++)
		if(*iLath)	delete(*iLath);

	// Release the reference to our points.
	m_pPoints->Release();
}


//------------------------------------------------------------------------------
/**
 *	Get a pointer to a lath referencing the specified facet index.
 *	The returned lath pointer can be any lath on the edge of the facet.
 *	Asserts if the facet index is invalid.
 *
 *	@param	iIndex	Index of the facet to query.
 *
 *	@return			Pointer to a lath on the facet.
 */
CqLath* CqSubdivision2::pFacet(TqInt iIndex)
{
	assert(iIndex < m_apFacets.size());
	return(m_apFacets[iIndex]);
}


//------------------------------------------------------------------------------
/**
 *	Get a pointer to a lath which references the specified vertex index.
 *	The returned lath pointer can be any lath which references the vertex.
 *	Asserts if the vertex index is invalid.
 *
 *	@param	iIndex	Index of the vertex to query.
 *
 *	@return			Pointer to a lath on the vertex.
 */
CqLath* CqSubdivision2::pVertex(TqInt iIndex)
{
	assert(iIndex < m_aapVertices.size() && m_aapVertices[iIndex].size() >= 1);
	return(m_aapVertices[iIndex][0]);
}


//------------------------------------------------------------------------------
/**
 *	Initialise the topology class to store the specified number of vertices.
 *	Use this function to prepare the topology structure to receive a number of
 *	vertices then use SetVertex to initialise them.
 *
 *	@param	cVerts	Then number of vertices that will be needed.
 */
void CqSubdivision2::Prepare(TqInt cVerts)
{
	// Initialise the array of vertex indexes to the appropriate size.
	m_aapVertices.resize(cVerts);

	m_fFinalised=TqFalse;
}


//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
TqInt CqSubdivision2::AddVertex(CqLath* pVertex)
{
	TqInt iIndex;

	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = pPoints()->aUserParams().begin(); iUP != pPoints()->aUserParams().end(); iUP++ )
	{
		iIndex = ( *iUP )->Size();
		( *iUP )->SetSize( iIndex+1 );

		switch ( ( *iUP )->Type() )
		{
				case type_float:
				{
					CqParameterTyped<TqFloat, TqFloat>* pParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );
					CreateVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_integer:
				{
					CqParameterTyped<TqInt, TqFloat>* pParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( ( *iUP ) );
					CreateVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_point:
				case type_normal:
				case type_vector:
				{
					CqParameterTyped<CqVector3D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
					CreateVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_color:
				{
					CqParameterTyped<CqColor, CqColor>* pParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );
					CreateVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_hpoint:
				{
					CqParameterTyped<CqVector4D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
					CreateVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_string:
				{
					//CqParameterTyped<CqString, CqString>* pParam = static_cast<CqParameterTyped<CqString, CqString>*>( ( *iUP ) );
					//CreateVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_matrix:
				{
					//CqParameterTyped<CqMatrix, CqMatrix>* pParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( ( *iUP ) );
					//CreateVertex( pParam, pVertex, iIndex );
				}
				break;
		}

	}

	// Resize the vertex lath 
	m_aapVertices.resize(iIndex+1);

	return(iIndex);
}


//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
TqInt CqSubdivision2::AddEdgeVertex(CqLath* pVertex)
{
	TqInt iIndex;

	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = pPoints()->aUserParams().begin(); iUP != pPoints()->aUserParams().end(); iUP++ )
	{
		iIndex = ( *iUP )->Size();
		( *iUP )->SetSize( iIndex+1 );

		switch ( ( *iUP )->Type() )
		{
				case type_float:
				{
					CqParameterTyped<TqFloat, TqFloat>* pParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );
					CreateEdgeVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_integer:
				{
					CqParameterTyped<TqInt, TqFloat>* pParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( ( *iUP ) );
					CreateEdgeVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_point:
				case type_normal:
				case type_vector:
				{
					CqParameterTyped<CqVector3D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
					CreateEdgeVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_color:
				{
					CqParameterTyped<CqColor, CqColor>* pParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );
					CreateEdgeVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_hpoint:
				{
					CqParameterTyped<CqVector4D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
					CreateEdgeVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_string:
				{
					//CqParameterTyped<CqString, CqString>* pParam = static_cast<CqParameterTyped<CqString, CqString>*>( ( *iUP ) );
					//CreateEdgeVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_matrix:
				{
					//CqParameterTyped<CqMatrix, CqMatrix>* pParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( ( *iUP ) );
					//CreateEdgeVertex( pParam, pVertex, iIndex );
				}
				break;
		}

	}

	// Resize the vertex lath 
	m_aapVertices.resize(iIndex+1);

	return(iIndex);
}


//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@return			The index of the new point.
 */
TqInt CqSubdivision2::AddFaceVertex(CqLath* pVertex)
{
	TqInt iIndex;

	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = pPoints()->aUserParams().begin(); iUP != pPoints()->aUserParams().end(); iUP++ )
	{
		iIndex = ( *iUP )->Size();
		( *iUP )->SetSize( iIndex+1 );

		switch ( ( *iUP )->Type() )
		{
				case type_float:
				{
					CqParameterTyped<TqFloat, TqFloat>* pParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );
					CreateFaceVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_integer:
				{
					CqParameterTyped<TqInt, TqFloat>* pParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( ( *iUP ) );
					CreateFaceVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_point:
				case type_normal:
				case type_vector:
				{
					CqParameterTyped<CqVector3D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
					CreateFaceVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_color:
				{
					CqParameterTyped<CqColor, CqColor>* pParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );
					CreateFaceVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_hpoint:
				{
					CqParameterTyped<CqVector4D, CqVector3D>* pParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
					CreateFaceVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_string:
				{
					//CqParameterTyped<CqString, CqString>* pParam = static_cast<CqParameterTyped<CqString, CqString>*>( ( *iUP ) );
					//CreateFaceVertex( pParam, pVertex, iIndex );
				}
				break;

				case type_matrix:
				{
					//CqParameterTyped<CqMatrix, CqMatrix>* pParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( ( *iUP ) );
					//CreateFaceVertex( pParam, pVertex, iIndex );
				}
				break;
		}

	}

	// Resize the vertex lath 
	m_aapVertices.resize(iIndex+1);

	return(iIndex);
}


//------------------------------------------------------------------------------
/**
 *	Add a new facet to the topology structure.
 *	Adds the facet by adding new laths for the specified vertex indices, and
 *	linking them to each other clockwise about the facet. By convention, as
 *	outside of the topology structure facets are stored counter clockwise, the
 *	vertex indices should be passed to this function as counter clockwise and
 *	they will be internally altered to specify the facet as clockwise.
 *
 *	@param	cVerts		The number of vertices in the facet.
 *	@param	pIndices	Pointer to an array of vertex indices.
 *
 *	@return				Pointer to one of the laths which represent this new
 *						facet in the topology structure.
 */
CqLath* CqSubdivision2::AddFacet(TqInt cVerts, TqInt* pIndices)
{
	CqLath* pLastLath=NULL;
	CqLath* pFirstLath=NULL;
	// Add the laths for this facet, referencing the appropriate vertexes as we go.
	for(TqInt iVert = 0; iVert < cVerts; iVert++)
	{
		CqLath* pNewLath = new CqLath();
		pNewLath->SetVertexIndex(pIndices[iVert]);

		if(NULL != pLastLath)
			pNewLath->SetpClockwiseFacet(pLastLath);

		m_apLaths.push_back(pNewLath);
		pLastLath = pNewLath;
		if(iVert == 0)	pFirstLath = pLastLath;

		// We also need to keep up to date a complete list of which laths refer to which 
		// vertices to aid us in finalising the topology structure later.
		m_aapVertices[pIndices[iVert]].push_back(pLastLath);
	}
	// complete the chain by linking the last one as the next clockwise one to the first.
	pFirstLath->SetpClockwiseFacet(pLastLath);

	// Add the start lath in as the one referring to this facet in the list.
	m_apFacets.push_back(pFirstLath);

	return(pFirstLath);
}


//------------------------------------------------------------------------------
/**
 *	Finalise the linkage of the laths.
 *	After adding vertices and facets, call this to complete the linkage of the
 *	laths. To overcome any non-manifold areas in the mesh, this function may
 *	change the topology in order to produce a manifold mesh, or series of
 *	manifold meshes. This also means that all facets in the mesh may no longer
 *	be joined in a complete loop, so care must be taken when traversing the
 *	topology to ensure that all facets are processed.
 */
void CqSubdivision2::Finalise()
{
	for(std::vector<std::vector<CqLath*> >::const_iterator ivert=m_aapVertices.begin(); ivert!=m_aapVertices.end(); ivert++)
	{
		TqInt cLaths = (*ivert).size();

		// If there is only one lath, it can't be connected to anything.
		if(cLaths<=1)	continue;

		// Create an array for the laths on this vertex that have been visited.
		std::vector<TqBool>  aVisited;
		aVisited.resize(cLaths);
		TqInt cVisited = 0;

		// Initialise it to all false.
		aVisited.assign(cLaths, TqFalse);
		
		CqLath* pCurrent = (*ivert)[0];
		CqLath* pStart = pCurrent;
		TqInt iCurrent = 0;
		TqInt iStart = 0;
		
		TqBool fDone = TqFalse;
		while(!fDone)
		{
			// Find a clockwise vertex match for the counterclockwise vertex index of this lath.
			TqInt ccwVertex = pCurrent->ccf()->VertexIndex();
			TqInt iLath = 0;
			for(iLath = 0; iLath < cLaths; iLath++)
			{
				// Only check non-visited laths.
				if(!aVisited[iLath] && (*ivert)[iLath]->cf()->VertexIndex() == ccwVertex)
				{
					pCurrent->SetpClockwiseVertex((*ivert)[iLath]);
					pCurrent = (*ivert)[iLath];
					iCurrent = iLath;
					// Mark the linked to lath as visited.
					aVisited[iLath] = TqTrue;
					cVisited++;

					break;
				}
			}
			// If we didn't find a match then we are done.
			fDone = iLath==cLaths;
		}

		// If the last lath wasn't linked, then we have a boundary condition, so
		// start again from the initial lath and process backwards.
		if(NULL == pCurrent->pClockwiseVertex())
		{
			fDone = TqFalse;
			while(!fDone)
			{
				// Find a counterclockwise vertex match for the clockwise vertex index of this lath.
				TqInt cwVertex = pStart->cf()->VertexIndex();
				TqInt iLath = 0;
				for(iLath = 0; iLath < cLaths; iLath++)
				{
					// Only check non-visited laths.
					if(!aVisited[iLath] && (*ivert)[iLath]->ccf()->VertexIndex() == cwVertex)
					{
						// Link the current to the match.
						(*ivert)[iLath]->SetpClockwiseVertex(pStart);
						// Mark the linked to lath as visited.
						aVisited[iStart] = TqTrue;
						cVisited++;
						pStart = (*ivert)[iLath];
						iStart = iLath;

						break;
					}
				}
				// If we didn't find a match then we are done.
				fDone = iLath==cLaths;
			}
		}
		aVisited[iStart] = TqTrue;
		cVisited++;
		// If we have not visited all the laths referencing this vertex, then we have a non-manifold situation.
		if(cVisited < cLaths)
		{
			return;
		}
	}
	
	m_fFinalised = TqTrue;
}


#define modulo(a, b) (a * b >= 0 ? a % b : (a % b) + b)
struct SqFaceLathList {
	CqLath* pA, *pB, *pC, *pD;
};

void CqSubdivision2::SubdivideFace(CqLath* pFace, std::vector<CqLath*>& apSubFaces)
{
	assert(NULL != pFace);

	// If this has already beed subdivided then skip it.
	if( NULL != pFace->pFaceVertex() )
	{
		apSubFaces.clear();
		std::vector<CqLath*> aQvf;
		pFace->pFaceVertex()->Qvf(aQvf);
		// Fill in the lath pointers to the same laths that reference the faces in the topology list. This ensures that
		// the dicing routine will still get the lath it expects in the corner for reading data out.
		std::vector<CqLath*>::iterator iVF;
		for( iVF = aQvf.begin(); iVF != aQvf.end(); iVF++ )
			apSubFaces.push_back( (*iVF)->ccf()->ccf() );
		return;
	}

	// First make sure that the appropriate neighbour facets have been subdivided if this is >0 level face.
	if( NULL != pFace->pParentFacet() )
	{
		std::vector<CqLath*> aQff;
		std::vector<CqLath*> apSubFaces2;
		pFace->pParentFacet()->Qff( aQff );
		std::vector<CqLath*>::iterator iF;
		for( iF = aQff.begin(); iF != aQff.end(); iF++ )
			SubdivideFace(*iF, apSubFaces2);
	}

	std::vector<CqLath*> aQfv;
	std::vector<TqInt> aVertices;

	pFace->Qfv(aQfv);
	TqInt n = aQfv.size();

	aVertices.resize((2*n)+1);

	// Clear the return array for subdface indices.
	apSubFaces.clear();

	// First of all setup the points.
	TqInt i;

	// Create new point for the face midpoint.
	TqInt iVert = AddFaceVertex(pFace);

	// Create new points for the edge midpoints.
	for(i = 0; i < n; i++)
	{
		TqInt iVert;
		// Create new vertices for the edge mid points.
		if( NULL == aQfv[i]->pMidVertex() )
			// Create new vertex for the edge midpoint.
			iVert = AddEdgeVertex(aQfv[i]);
		else
			// There is already a next level vertex for this, so just setup a lath to it.
			iVert = aQfv[i]->pMidVertex()->VertexIndex();

		// Store the index, for later lath creation
		aVertices[i+n] = iVert;
	}

	// Create new points for the existing vertices
	for(i = 0; i < n; i++)
	{
		TqInt iVert;
		// Create new vertices for the original points.
		if( NULL == aQfv[i]->pChildVertex() )
			// Create a new vertex for the next level
			iVert = AddVertex(aQfv[i]);
		else
			// There is already a next level vertex for this, so just setup a lath to it.
			iVert = aQfv[i]->pChildVertex()->VertexIndex();

		// Store the index, for later lath creation
		aVertices[i] = iVert;
	}

	// Store the index, for later lath creation
	aVertices[2*n] = iVert;

	// Now create new laths for the new facets
	std::vector<SqFaceLathList>	apFaceLaths;
	apFaceLaths.resize(n);

	for( i = 0; i < n; i++ )
	{
		// For each facet, create 4 laths and join them in the order of the facet
		CqLath* pLathA = new CqLath();
		m_apLaths.push_back(pLathA);
		CqLath* pLathB = new CqLath();
		m_apLaths.push_back(pLathB);
		CqLath* pLathC = new CqLath();
		m_apLaths.push_back(pLathC);
		CqLath* pLathD = new CqLath();
		m_apLaths.push_back(pLathD);
		pLathA->SetVertexIndex(aVertices[i]);
		pLathB->SetVertexIndex(aVertices[(modulo((i+1),n))+n]);
		pLathC->SetVertexIndex(aVertices[2*n]);
		pLathD->SetVertexIndex(aVertices[i+n]);
		pLathA->SetpClockwiseFacet(pLathB);
		pLathB->SetpClockwiseFacet(pLathC);
		pLathC->SetpClockwiseFacet(pLathD);
		pLathD->SetpClockwiseFacet(pLathA);
		pLathA->SetpParentFacet(pFace);
		pLathB->SetpParentFacet(pFace);
		pLathC->SetpParentFacet(pFace);
		pLathD->SetpParentFacet(pFace);

		apFaceLaths[i].pA = pLathA;
		apFaceLaths[i].pB = pLathB;
		apFaceLaths[i].pC = pLathC;
		apFaceLaths[i].pD = pLathD;

		// Fill in the vertex references table for these vertices.
		m_aapVertices[pLathA->VertexIndex()].push_back(pLathA);
		m_aapVertices[pLathB->VertexIndex()].push_back(pLathB);
		m_aapVertices[pLathC->VertexIndex()].push_back(pLathC);
		m_aapVertices[pLathD->VertexIndex()].push_back(pLathD);

		// Set the child vertex pointer for all laths which reference the A vertex of this facet
		// so that we can use them when subdividing other faces.
		CqLath* pNextV = aQfv[i];
		do
		{
			pNextV->SetpChildVertex(pLathA);
			pNextV = pNextV->pClockwiseVertex();
		}while( pNextV && pNextV != aQfv[i]);
		
		// For this edge of the original face, set a ponter to the new midpoint lath, so that we can
		// use it when subdividing neighbour facets, do the same for the lath representing the edge in the
		// other direction if not a boundary.
		aQfv[i]->SetpMidVertex(pLathD);
		if( NULL != aQfv[i]->ec() )	
			aQfv[i]->ec()->SetpMidVertex(pLathD);

		// Store a lath reference for the facet.
		apSubFaces.push_back(pLathA);
		m_apFacets.push_back(pLathA);
	}

	// Now connect up the laths we have created.
	// The clcckwise face connections will have already been made, we need to fixup and clockwise
	// vertex connections we can.
	for( i = 0; i < n; i++ )
	{
		// Set the facet point reference for all laths representing this facet.
		aQfv[i]->SetpFaceVertex(apFaceLaths[i].pC);
		// Connect midpoints clockwise vertex pointers.
		apFaceLaths[((i+1)%n)].pD->SetpClockwiseVertex( apFaceLaths[i].pB );
		// Connect all laths around the new face point.
		apFaceLaths[i].pC->SetpClockwiseVertex( apFaceLaths[ ((i+1)%n) ].pC );
		// Connect the new corner vertices, this is only possible if neighbouring facets have previously been
		// subdivided.
		std::vector<CqLath*>::iterator iVertLath;
		for( iVertLath = m_aapVertices[apFaceLaths[i].pA->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pA->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->cf()->VertexIndex() == apFaceLaths[i].pD->VertexIndex() )
				apFaceLaths[i].pA->SetpClockwiseVertex( (*iVertLath ) );
			else if( (*iVertLath)->ccf()->VertexIndex() == apFaceLaths[i].pB->VertexIndex() )
				(*iVertLath)->SetpClockwiseVertex( apFaceLaths[i].pA );
		}
		// Connect the new edge midpoint vertices to any neighbours, this is only possible if neighbouring facets have previously been
		// subdivided.
		for( iVertLath = m_aapVertices[apFaceLaths[i].pB->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pB->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->cf()->VertexIndex() == apFaceLaths[i].pA->VertexIndex() )
				apFaceLaths[i].pB->SetpClockwiseVertex( (*iVertLath ) );
		}
		for( iVertLath = m_aapVertices[apFaceLaths[i].pD->VertexIndex()].begin(); iVertLath != m_aapVertices[apFaceLaths[i].pD->VertexIndex()].end(); iVertLath++ )
		{
			if( (*iVertLath)->ccf()->VertexIndex() == apFaceLaths[i].pA->VertexIndex() )
				(*iVertLath )->SetpClockwiseVertex( apFaceLaths[i].pD );
		}
	}
}


void CqSubdivision2::OutputMesh(const char* fname, std::vector<CqLath*>* paFaces)
{
	std::ofstream file(fname);
	std::vector<CqLath*> aQfv;

	TqInt i;
	for( i = 0; i < cVertices(); i++ )
	{
		CqVector3D vec = pPoints()->P()->pValue()[ pVertex( i )->VertexIndex() ];
		file << "v " << vec.x() << " " << vec.y() << " " << vec.z() << std::endl;
	}
	

	for(i = 0; i < cFacets(); i++)
	{
		if( NULL == pFacet(i)->pFaceVertex())
		{
			pFacet(i)->Qfv(aQfv);
			TqInt j;
			file << "f ";
			for( j = 0; j < aQfv.size(); j++ )
				file << aQfv[j]->VertexIndex()+1 << " ";
			file << std::endl;
		}
	}

	if( NULL != paFaces)
	{
		file << "g CurrentFace" << std::endl;
		for(i = 0; i < paFaces->size(); i++)
		{
			(*paFaces)[i]->Qfv(aQfv);
			TqInt j;
			file << "f ";
			for( j = 0; j < aQfv.size(); j++ )
				file << aQfv[j]->VertexIndex()+1 << " ";
			file << std::endl;
		}
	}

	file.close();
}


void CqSubdivision2::OutputInfo(const char* fname)
{
	std::ofstream file(fname);
	std::vector<CqLath*> aQfv;

	for(TqInt i = 0; i < cLaths(); i++)
	{
		CqLath* pL = apLaths()[i];
		file << i << " - " << 
			(char)(pL->ID()+'A') << " - "	<<
			pL->VertexIndex() << " - (cf) ";
		if( pL->cf() )
			file << (char)(pL->cf()->ID()+'A');
		else
			file << "***";
		file << " - (cv) ";
		
		if(pL->cv())
			file << (char)(pL->cv()->ID()+'A');
		else
			file << "***";
			
		file << std::endl;
	}

	file.close();
}


CqBound	CqSurfaceSubdivisionPatch::Bound() const
{
	assert( NULL != pTopology() );
	assert( NULL != pTopology()->pPoints() );
	assert( NULL != pFace() );

	// First make sure that the appropriate neighbour facets have been subdivided if this is >0 level face.
	if( NULL != pFace()->pParentFacet() )
	{
		std::vector<CqLath*> aQff;
		std::vector<CqLath*> apSubFaces2;
		pFace()->pParentFacet()->Qff( aQff );
		std::vector<CqLath*>::iterator iF;
		for( iF = aQff.begin(); iF != aQff.end(); iF++ )
			pTopology()->SubdivideFace(*iF, apSubFaces2);
	}

	CqBound B;
	
	// Get the laths of the surrounding faces.
	std::vector<CqLath*> aQff;
	pFace()->Qff(aQff);
	std::vector<CqLath*>::iterator iFF;
	for( iFF = aQff.begin(); iFF != aQff.end(); iFF++ )
	{
		// Get the laths that reference the vertices of this face
		std::vector<CqLath*> aQfv;
		(*iFF)->Qfv(aQfv);

		// Now get the vertices, and form the bound.
		std::vector<CqLath*>::iterator iQfv;
		for( iQfv = aQfv.begin(); iQfv != aQfv.end(); iQfv++ )
			B.Encapsulate(pTopology()->pPoints()->P()->pValue((*iQfv)->VertexIndex())[0]);
	}

	return(B);
}

CqMicroPolyGridBase* CqSurfaceSubdivisionPatch::Dice()
{
	assert( NULL != pTopology() );
	assert( NULL != pTopology()->pPoints() );
	assert( NULL != pFace() );

	TqInt dicesize = MAX(m_uDiceSize, m_vDiceSize);
	TqInt sdcount = ( dicesize == 16 ) ? 4 : ( dicesize == 8 ) ? 3 : ( dicesize == 4 ) ? 2 : ( dicesize == 2 ) ? 1 : 1;
	dicesize = 1 << sdcount;

	CqMicroPolyGrid* pGrid = new CqMicroPolyGrid( dicesize, dicesize, pTopology()->pPoints() );

	TqInt isd;
	std::vector<CqLath*> apSubFace1, apSubFace2;
	apSubFace1.push_back(pFace());
	for( isd = 0; isd < sdcount; isd++ )
	{
		apSubFace2.clear();
		std::vector<CqLath*>::iterator iSF;
		for( iSF = apSubFace1.begin(); iSF != apSubFace1.end(); iSF++ )
		{
			// Subdivide this face, storing the resulting new face indices.
			std::vector<CqLath*> apSubFaceTemp;
			pTopology()->SubdivideFace( (*iSF), apSubFaceTemp );
			// Now combine these into the new face indices for this subdivision level.
			apSubFace2.insert(apSubFace2.end(), apSubFaceTemp.begin(), apSubFaceTemp.end());
		}
		// Now swap the new level's indices for the old before repeating at the next level, if appropriate.
		apSubFace1.swap(apSubFace2);
	}

	// Now we use the first face index to start our extraction
	TqInt nc, nr, c, r;
	nc = nr = MAX(m_uDiceSize, m_vDiceSize);
	r = 0;

	CqLath* pLath, *pTemp;
	pLath = apSubFace1[0];
	pTemp = pLath;

	// Get data from pLath
	TqInt ivA = pLath->VertexIndex();
	TqInt indexA = 0;
	
	StoreDice( pGrid, ivA, indexA );

	indexA++;		
	pLath = pLath->ccf();
	c = 0;
	while( c < nc )
	{
		TqInt ivA = pLath->VertexIndex();
		StoreDice( pGrid, ivA, indexA );

		if( c < ( nc - 1 ) )	
			pLath = pLath->cv()->ccf();
		
		indexA++;
		c++;
	}
	r++;

	while( r <= nr )
	{
		pLath = pTemp->cf();
		pTemp = pLath->ccv();

		// Get data from pLath
		TqInt ivA = pLath->VertexIndex();
		TqInt indexA = ( r * ( nc + 1 ) );
		StoreDice( pGrid, ivA, indexA );

		indexA++;		
		pLath = pLath->cf();
		c = 0;
		while( c < nc )
		{
			TqInt ivA = pLath->VertexIndex();
			StoreDice( pGrid, ivA, indexA );

			if( c < ( nc - 1 ) )	
				pLath = pLath->ccv()->cf();
			
			indexA++;
			c++;
		}

		r++;
	}

	TqInt lUses = Uses();

	// If the color and opacity are not defined, use the system values.
	if ( USES( lUses, EnvVars_Cs ) && !pTopology()->pPoints()->bHasCs() )
	{
		if ( NULL != pAttributes() ->GetColorAttribute( "System", "Color" ) )
			pGrid->Cs() ->SetColor( pAttributes() ->GetColorAttribute( "System", "Color" ) [ 0 ] );
		else
			pGrid->Cs() ->SetColor( CqColor( 1, 1, 1 ) );
	}

	if ( USES( lUses, EnvVars_Os ) && !pTopology()->pPoints()->bHasOs() )
	{
		if ( NULL != pAttributes() ->GetColorAttribute( "System", "Opacity" ) )
			pGrid->Os() ->SetColor( pAttributes() ->GetColorAttribute( "System", "Opacity" ) [ 0 ] );
		else
			pGrid->Os() ->SetColor( CqColor( 1, 1, 1 ) );
	}

	return( pGrid );
}



static void StoreDiceAPVar( IqShader* pShader, CqParameter* pParam, TqUint ivA, TqUint indexA )
{
	// Find the argument
	IqShaderData * pArg = pShader->FindArgument( pParam->strName() );
	if ( NULL != pArg )
	{
		switch ( pParam->Type() )
		{
				case type_float:
				{
					CqParameterTyped<TqFloat, TqFloat>* pNParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParam );
					pArg->SetValue( *pNParam->pValue( ivA ), indexA );
				}
				break;

				case type_integer:
				{
					CqParameterTyped<TqInt, TqFloat>* pNParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParam );
					pArg->SetValue( *pNParam->pValue( ivA ), indexA );
				}
				break;

				case type_point:
				case type_vector:
				case type_normal:
				{
					CqParameterTyped<CqVector3D, CqVector3D>* pNParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParam );
					pArg->SetValue( *pNParam->pValue( ivA ), indexA );
				}
				break;

				case type_hpoint:
				{
					CqParameterTyped<CqVector4D, CqVector3D>* pNParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParam );
					pArg->SetValue( *pNParam->pValue( ivA ), indexA );
				}
				break;

				case type_string:
				{
					CqParameterTyped<CqString, CqString>* pNParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParam );
					pArg->SetValue( *pNParam->pValue( ivA ), indexA );
				}
				break;

				case type_color:
				{
					CqParameterTyped<CqColor, CqColor>* pNParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParam );
					pArg->SetValue( *pNParam->pValue( ivA ), indexA );
				}
				break;

				case type_matrix:
				{
					CqParameterTyped<CqMatrix, CqMatrix>* pNParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParam );
					pArg->SetValue( *pNParam->pValue( ivA ), indexA );
				}
				break;
		}
	}
}


void CqSurfaceSubdivisionPatch::StoreDice( CqMicroPolyGrid* pGrid, TqInt iParam, TqInt iData )
{
	TqInt lUses = Uses();
	
	if ( USES( lUses, EnvVars_P ) )
		pGrid->P() ->SetPoint( ( *pTopology()->pPoints()->P() ) [ iParam ], iData );

	if ( USES( lUses, EnvVars_s ) && ( NULL != pGrid->s() ) && pTopology()->pPoints()->bHass() )
		pGrid->s() ->SetFloat( ( *pTopology()->pPoints()->s() ) [ iParam ], iData );

	if ( USES( lUses, EnvVars_t ) && ( NULL != pGrid->t() ) && pTopology()->pPoints()->bHast() )
		pGrid->t() ->SetFloat( ( *pTopology()->pPoints()->t() ) [ iParam ], iData );

	if ( USES( lUses, EnvVars_u ) && ( NULL != pGrid->u() ) && pTopology()->pPoints()->bHasu() )
		pGrid->u() ->SetFloat( ( *pTopology()->pPoints()->u() ) [ iParam ], iData );

	if ( USES( lUses, EnvVars_v ) && ( NULL != pGrid->v() ) && pTopology()->pPoints()->bHasv() )
		pGrid->v() ->SetFloat( ( *pTopology()->pPoints()->v() ) [ iParam ], iData );

	// Now lets store the diced user specified primitive variables.
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = pTopology()->pPoints()->aUserParams().begin(); iUP != pTopology()->pPoints()->aUserParams().end(); iUP++ )
	{
		/// \todo: Must transform point/vector/normal/matrix parameter variables from 'object' space to current before setting.
		if ( NULL != pGrid->pAttributes() ->pshadSurface() )
			StoreDiceAPVar( pGrid->pAttributes()->pshadSurface(), ( *iUP ), iParam, iData );

		if ( NULL != pGrid->pAttributes() ->pshadDisplacement() )
			StoreDiceAPVar( pGrid->pAttributes()->pshadDisplacement(), ( *iUP ), iParam, iData );

		if ( NULL != pGrid->pAttributes() ->pshadAtmosphere() )
			StoreDiceAPVar( pGrid->pAttributes()->pshadAtmosphere(), ( *iUP ), iParam, iData );
	}
}


TqInt CqSurfaceSubdivisionPatch::Split( std::vector<CqBasicSurface*>& aSplits )
{
	assert( NULL != pTopology() );
	assert( NULL != pTopology()->pPoints() );
	assert( NULL != pFace() );

	// If the patch is a quad with each corner having valence 4, and no special features, 
	// we can just create a B-Spline patch.
	TqBool fCanUsePatch = TqFalse;
	std::vector<CqLath*> aQfv, aQvv;
	pFace()->Qfv(aQfv);
	if( aQfv.size() == 4 )
	{
		aQfv[0]->Qvv( aQvv );
		if( aQvv.size() == 4 )
		{
			aQfv[1]->Qvv( aQvv );
			if( aQvv.size() == 4 )
			{
				aQfv[2]->Qvv( aQvv );
				if( aQvv.size() == 4 )
				{
					aQfv[3]->Qvv( aQvv );
					if( aQvv.size() == 4 )
						fCanUsePatch = TqTrue;
				}
			}
		}
	}
	
	if( fCanUsePatch )
	{
		// Create a surface patch
		CqSurfacePatchBicubic * pSurface = new CqSurfacePatchBicubic();
		pSurface->AddRef();
		// Fill in default values for all primitive variables not explicitly specified.
		pSurface->SetSurfaceParameters( *pTopology()->pPoints() );

		CqLath* pPoint = pFace()->cv()->cv()->cf()->cf();
		CqLath* pRow = pPoint;

		std::vector<TqInt>	aiVertices;

		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccv()->cf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pRow = pPoint = pRow->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pRow = pPoint = pRow->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pRow->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );
		pPoint = pPoint->cv()->ccf();
		aiVertices.push_back( pPoint->VertexIndex() );

		pSurface->AddPrimitiveVariable( new CqParameterTypedVertex<CqVector4D, type_hpoint, CqVector3D>( "P", 0 ) );
		pSurface->P() ->SetSize( pSurface->cVertex() );

		TqInt i;
		for( i = 0; i < 16; i++ )
		{
			CqVector3D vA = (*pTopology()->pPoints()->P())[ aiVertices[i] ];
			( *pSurface->P() ) [i] = vA;
		}
		pSurface->ConvertToBezierBasis();

		aSplits.push_back(pSurface);
	}
	else
	{
		// Subdivide the face, and create new patches for the subfaces.
		std::vector<CqLath*> apSubFaces;
		pTopology()->SubdivideFace( pFace(), apSubFaces );

		// Now create new patch objects for each subface.
		std::vector<CqLath*>::iterator iSF;
		for( iSF = apSubFaces.begin(); iSF != apSubFaces.end(); iSF++ )
		{
			CqSurfaceSubdivisionPatch* pNew = new CqSurfaceSubdivisionPatch( pTopology(), (*iSF) );
			aSplits.push_back(pNew);
		}
	}

	return(aSplits.size());
}

TqBool CqSurfaceSubdivisionPatch::Diceable()
{
	assert( NULL != pTopology() );
	assert( NULL != pTopology()->pPoints() );
	assert( NULL != pFace() );

	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( TqFalse );

	// Get the laths that reference the vertices of this face
	std::vector<CqLath*> aQfv;
	pFace()->Qfv(aQfv);

	// Cannot dice if not 4 points
	if ( aQfv.size() != 4 )
		return ( TqFalse );

	// Otherwise we should continue to try to find the most advantageous split direction, OR the dice size.
	const CqMatrix & matCtoR = QGetRenderContext() ->matSpaceToSpace( "camera", "raster" );

	// Convert the control hull to raster space.
	CqVector2D	avecHull[ 4 ];
	TqInt i;

	TqFloat ShadingRate = pAttributes() ->GetFloatAttribute( "System", "ShadingRate" ) [ 0 ];

	for ( i = 0; i < 4; i++ )
		avecHull[ i ] = matCtoR * pTopology()->pPoints()->P()->pValue() [ aQfv[ i ]->VertexIndex() ];

	TqFloat uLen = 0;
	TqFloat vLen = 0;

	CqVector2D	Vec1 = avecHull[ 1 ] - avecHull[ 0 ];
	CqVector2D	Vec2 = avecHull[ 2 ] - avecHull[ 3 ];
	uLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	Vec1 = avecHull[ 3 ] - avecHull[ 0 ];
	Vec2 = avecHull[ 1 ] - avecHull[ 1 ];
	vLen = ( Vec1.Magnitude2() > Vec2.Magnitude2() ) ? Vec1.Magnitude2() : Vec2.Magnitude2();

	uLen = sqrt( uLen / ShadingRate);
	vLen = sqrt( vLen / ShadingRate);

	m_SplitDir = ( uLen > vLen ) ? SplitDir_U : SplitDir_V;

	// TODO: Should ensure powers of half to prevent cracking.
	uLen = MAX( ROUND( uLen ), 1 );
	vLen = MAX( ROUND( vLen ), 1 );
	
	m_uDiceSize = static_cast<TqInt>( uLen );
	m_vDiceSize = static_cast<TqInt>( vLen );

	// Ensure power of 2 to avoid cracking
	m_uDiceSize = CEIL_POW2( m_uDiceSize );
	m_vDiceSize = CEIL_POW2( m_vDiceSize );

	if ( uLen < FLT_EPSILON || vLen < FLT_EPSILON )
	{
		m_fDiscard = TqTrue;
		return ( TqFalse );
	}

	TqFloat gs = SqrtGridSize();

	if ( m_uDiceSize > gs) return TqFalse;
	if ( m_vDiceSize > gs) return TqFalse;

	return ( TqTrue );
}


END_NAMESPACE( Aqsis )
