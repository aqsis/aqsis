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

#include	<fstream>
#include	<vector>

START_NAMESPACE( Aqsis )

//------------------------------------------------------------------------------
/**
 *	Constructor.
 */

CqSubdivision2::CqSubdivision2() : m_fFinalised(TqFalse)
{
}


//------------------------------------------------------------------------------
/**
 *	Destructor.
 */

CqSubdivision2::~CqSubdivision2()
{
	// Delete the array of laths generated during the facet adding phase.
	for(std::vector<CqLath*>::const_iterator iLath=apLaths().begin(); iLath!=apLaths().end(); iLath++)
		if(*iLath)	delete(*iLath);
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
	assert(iIndex < m_aapVertices.size() && 1 >= m_aapVertices[iIndex].size());
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
	m_aVertices.resize(cVerts);

	m_fFinalised=TqFalse;
}


//------------------------------------------------------------------------------
/**
 *	Set the contents of a specified vertex index.
 *	Used after calling Prepare to setup the initial state of the vertex
 *	positions. Asserts if the index is out of range.
 *
 *	@param	iIndex	Index of the vertex to set.
 *	@param	vecVert	Position to set the vertex to.
 */
void CqSubdivision2::SetVertex(TqInt iIndex, CqVector3D vecVert)
{
	assert(iIndex < m_aVertices.size());
	m_aVertices[iIndex] = vecVert;
}


//------------------------------------------------------------------------------
/**
 *	Add a completely new vertex to the list.
 *	Appends a new vertex to the end of the list, updating the referencing
 *	table as well.
 *
 *	@param	vecVert	Position to apply to the new vertex.
 *
 *	@return			The index of the new point.
 */
TqInt CqSubdivision2::AddVertex(CqVector3D vecVert)
{
	TqInt iIndex = m_aVertices.size();
	m_aVertices.push_back(vecVert);

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

void CqSubdivision2::SubdivideFace(TqInt iF)
{
	assert(iF < cFacets());

	std::vector<CqLath*> aQfv;
	std::vector<TqInt> aVertices;

	pFacet(iF)->Qfv(aQfv);
	TqInt n = aQfv.size();

	aVertices.resize((2*n)+1);

	// First of all setup the points.
	TqInt i;
	CqVector3D vF(0,0,0);
	for(i = 0; i < n; i++)
	{
		TqInt iVert;
		// Create new vertices for the original points.
		if( NULL == aQfv[i]->pChildVertex() )
		{
			// Create a new vertex for the next level
			iVert = AddVertex(m_aVertices[aQfv[i]->VertexIndex()]);
			m_aapVertices.resize(iVert+1);
		}
		else
		{
			// There is already a next level vertex for this, so just setup a lath to it.
			iVert = aQfv[i]->pChildVertex()->VertexIndex();
		}

		// Store the index, for later lath creation
		aVertices[i] = iVert;
		vF += m_aVertices[aQfv[i]->VertexIndex()];
	}

	for(i = 0; i < n; i++)
	{
		TqInt iVert;
		// Create new vertices for the edge mid points.
		if( NULL == aQfv[i]->pMidVertex() )
		{
			// Create new vertex for the edge midpoint.
			CqVector3D vH = m_aVertices[aQfv[i]->VertexIndex()];
			CqVector3D vT = m_aVertices[aQfv[i]->ccf()->VertexIndex()];
			iVert = AddVertex((vH+vT)/2);
			m_aapVertices.resize(iVert+1);
		}
		else
		{
			// There is already a next level vertex for this, so just setup a lath to it.
			iVert = aQfv[i]->pMidVertex()->VertexIndex();
		}

		// Store the index, for later lath creation
		aVertices[i+n] = iVert;
	}

	// Create new vertex for the facepoint.
	TqInt iVert = AddVertex(vF/n);
	m_aapVertices.resize(iVert+1);

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
		m_apFacets.push_back(pLathA);
	}

	// Now connect up the laths we have created.
	// The clcckwise face connections will have already been made, we need to fixup and clockwise
	// vertex connections we can.
	for( i = 0; i < n; i++ )
	{
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


void CqSubdivision2::OutputMesh(const char* fname)
{
	std::ofstream file(fname);
	std::vector<CqLath*> aQfv;

	for(TqInt i = 0; i < cFacets(); i++)
	{
		pFacet(i)->Qfv(aQfv);
		CqVector3D vA = m_aVertices[aQfv[0]->VertexIndex()];
		TqInt j=1;
		while( j < aQfv.size()-1 )
		{
			CqVector3D vB = m_aVertices[aQfv[j++]->VertexIndex()];
			CqVector3D vC = m_aVertices[aQfv[j]->VertexIndex()];
			file << vA.x() << " " << vA.y() << " " << vA.z() << " " << 
					vB.x() << " " << vB.y() << " " << vB.z() << " " << 
					vC.x() << " " << vC.y() << " " << vC.z() << " " << std::endl;
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



END_NAMESPACE( Aqsis )
