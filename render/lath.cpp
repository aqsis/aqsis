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
		\brief Implements classes for storing mesh topology information..
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"lath.h"


START_NAMESPACE( Aqsis )

DEFINE_STATIC_MEMORYPOOL( CqLath, 512 );

//------------------------------------------------------------------------------
/**
 *	Get the next lath clockwise around the facet.
 *	Get a pointer to the next lath in a clockwise direction around the
 *	associated facet. This information is inherent in the data structures.
 *
 *	@return	Pointer to the lath.
 */
CqLath* CqLath::cf() const
{
	// Inherent in the data structure.
	return(m_pClockwiseFacet);
}


//------------------------------------------------------------------------------
/**
 *	Get the next lath clockwise about the vertex.
 *	Get a pointer to the next lath in a clockwise direction about the
 *	associated vertex. This information is inherent in the data structure.
 *
 *	@return	Pointer to the lath.
 */
CqLath* CqLath::cv() const
{
	// Inherent in the data strucure.
	return(m_pClockwiseVertex);
}


//------------------------------------------------------------------------------
/**
 *	Get the edge companion lath.
 *	Get a pointer to the lath which represents the same edge but in the
 *	opposite direction, i.e. refers to the opposite vertex.
 *
 *	@return	Pointer to the lath.
 */
CqLath* CqLath::ec() const
{
	// If the associated edge is boundary there is no companion.
	assert(NULL != cf());
	if(NULL != cv())
		return(cv()->cf());
	else
		return(NULL);
}

//------------------------------------------------------------------------------
/**
 *	Get the lath counter clockwise about the facet.
 *	Get a pointer to the next lath in a counter clockwise direction about the
 *	associated facet. This function is constant in all cases excepth where the
 *	associated edge is a boundary edge, in which case it is linear in the
 *	number of edges in the associated facet.
 *
 *	@return	Pointer to the lath.
 */
CqLath* CqLath::ccf() const
{
	// If the associated edge is boundary, we will need to search backwards.
	if(NULL != ec() && NULL != ec()->cv())
		return(ec()->cv());
	else
	{
		CqLath* pLdash=cf();
		while(this != pLdash->cf() && NULL != pLdash->cf())
			pLdash=pLdash->cf();
		assert(this == pLdash->cf());
		return(pLdash);
	}
}


//------------------------------------------------------------------------------
/**
 *	Get the lath counter clockwise about the vertex.
 *	Get a pointer to the next lath in a counter clockwise direction about the
 *	associated vertex. This function is constant in all cases.
 *
 *	@return	Pointer to the lath.
 */
CqLath* CqLath::ccv() const
{
	// If the associated edge is boundary, we will need to search backwards.
	assert(NULL != cf());
	if(NULL != cf()->ec())
		return(cf()->ec());
	else
		return(NULL);
}


//------------------------------------------------------------------------------
/**
 *	Get the faces surrounding an edge.
 *	Get a list of laths representing the faces surrounding an edge, will
 *	return just one if the edge is a boundary.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qef(std::vector<CqLath*>& Result) 
{
	Result.clear();
	// Laths representing the two faces bounding an edge are given by L and L->ec(). If edge
	// is a boundary, only L is passed back.
	CqLath *pTmpLath = this;
	Result.push_back(pTmpLath);

	if(NULL != ec())
		Result.push_back(ec());
}


//------------------------------------------------------------------------------
/**
 *	Get the vertices surounding an edge.
 *	Get a list of laths representing the vertices making up an edge.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qev(std::vector<CqLath*>& Result) 
{
	Result.clear();
	// Laths representing the two vertices of the associated edge are given by
	// L and L->ccf(). Note we use cf here because itis guarunteed, whereas ec is not.
	CqLath *pTmpLath = this;
	Result.push_back(pTmpLath);
	Result.push_back(ccf());
}


//------------------------------------------------------------------------------
/**
 *	Get the edges surrounding a facet.
 *	Get a list of laths representing the esges making up a facet.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qfe(std::vector<CqLath*>& Result) 
{
	Result.clear();
	// Laths representing the edges of the associated facet are obtained by following
	// clockwise links around the face.
	CqLath *pTmpLath = this;
	Result.push_back(pTmpLath);

	CqLath* pNext = cf();
	while(this != pNext)
	{
		assert(NULL != pNext);
		Result.push_back(pNext);
		pNext = pNext->cf();
	}
}


//------------------------------------------------------------------------------
/**
 *	Get the edges emanating from a vertex.
 *	Get a list of laths representing the edges which emanate from the vertex
 *	this lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qve(std::vector<CqLath*>& Result) 
{
	Result.clear();
	// Laths representing the edges that radiate from the associated vertex are obtained by 
	// following the clockwise vertex links around the vertex. 
	CqLath *pTmpLath = this;
	Result.push_back(pTmpLath);

	CqLath* pNext = cv();
	CqLath* pLast = this;
	while(NULL != pNext && this != pNext)
	{
		Result.push_back(pNext);
		pLast = pNext;
		pNext = pNext->cv();
	}

	// If we hit a boundary, add the ec of this boundary edge and start again going backwards.
	// @warning Adding ccf for the boundary edge means that the lath represents a different vertex.
	if(NULL == pNext)
	{
		pLast = this;
		pNext = ccv();
		// We know we are going to hit a boundary in this direction as well so we can just look for that
		// case as a terminator.
		while(NULL != pNext)
		{
			assert( pNext != this );
			Result.push_back(pNext);
			pLast = pNext;
			pNext = pNext->ccv();
		}
		// We have hit the boundary going the other way, so add the ccf of this boundary edge.
		Result.push_back(pLast->cf());
	}
}


//------------------------------------------------------------------------------
/**
 *	Get the vertices surrounding a facet.
 *	Get a list of laths representing the vertices which make up the facet this
 *	lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qfv(std::vector<CqLath*>& Result)
{
	Qfe(Result);
}


//------------------------------------------------------------------------------
/**
 *	Get the vertices emanating from this vertex.
 *	Get a list of laths representing the vertices emanating from the vertex
 *	this lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qvv(std::vector<CqLath*>& Result)
{
	Qve(Result);

	// We can get the laths for the vertices surrounding a vertex by getting the cf() for each
	// lath in Qev. Note we must check first if the lath in Qve represents the same vertex as this
	// as if there is a boundary case, the lath on the clockwise boundary will already point to the 
	// opposite vertex.
	for(std::vector<CqLath*>::iterator iLath = Result.begin(); iLath!=Result.end(); iLath++)
	{
		if((*iLath)->VertexIndex() == VertexIndex())
			(*iLath) = (*iLath)->ccf();
	}
}


//------------------------------------------------------------------------------
/**
 *	Get the facets which share this vertex.
 *	Get a list of laths which represent the facets which share the vertex this
 *	lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qvf(std::vector<CqLath*>& Result)
{
	Result.clear();

	// Laths representing the edges that radiate from the associated vertex are obtained by 
	// following the clockwise vertex links around the vertex. 
	CqLath *pTmpLath = this;
	Result.push_back(pTmpLath);

	CqLath* pNext = cv();
	while(NULL != pNext && this != pNext)
	{
		Result.push_back(pNext);
		pNext = pNext->cv();
	}

	// If we hit a boundary, start again going backwards.
	if(NULL == pNext)
	{
		pNext = ccv();
		// We know we are going to hit a boundary in this direction as well so we can just look for that
		// case as a terminator.
		while(NULL != pNext)
		{
			Result.push_back(pNext);
			pNext = pNext->ccv();
		}
	}
}


//------------------------------------------------------------------------------
/**
 *	Get the edges emanating from this edge.
 *	Get a list of laths which represent the edges which share a vertex with
 *	the edge this lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qee(std::vector<CqLath*>& Result)
{
	Result.clear();
	std::vector<CqLath*> ResQve1;
	Qve(ResQve1);
	std::vector<CqLath*> ResQve2;
	ccf()->Qve(ResQve2);

	// The laths representing the edges radiating from the two vertices of the edge this lath represents
	// can be implemented by taking the union of Qve for this and cf() and removing the duplicate cf() if
	// it exists.
	Result.insert(Result.end(), ResQve1.begin(), ResQve1.end());

	for(std::vector<CqLath*>::iterator iLath = ResQve2.begin(); iLath!=ResQve2.end(); iLath++)
	{
		if(ec() != (*iLath) && this != (*iLath))
			Result.push_back((*iLath));
	}
}


//------------------------------------------------------------------------------
/**
 *	Get the facets which surround this facet.
 *	Get a list of laths which represent the faces which share either a vertex
 *	or an edge with the facet this lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qff(std::vector<CqLath*>& Result)
{
	Result.clear();
	std::vector<CqLath*> ResQfe;
	Qfe(ResQfe);

	// The laths representing the edges radiating from the two vertices of the edge this lath represents
	// can be implemented by taking the union of Qve for this and cf() and removing the duplicate ec() if
	// it exists.
	for(std::vector<CqLath*>::iterator iLath = ResQfe.begin(); iLath!=ResQfe.end(); iLath++)
	{
		std::vector<CqLath*> ResQev;
		(*iLath)->Qve(ResQev);
		for(std::vector<CqLath*>::iterator iEdge = ResQev.begin(); iEdge!=ResQev.end(); iEdge++)
		{
			CqLath* pNew = (*iEdge);
			// Search for the new candidate by traversing the cf lists for all laths currrently in the
			// result list.
			TqBool fValid = TqTrue;
			for(std::vector<CqLath*>::iterator iCurr = Result.begin(); iCurr!=Result.end(); iCurr++)
			{
				CqLath* pVisited =(*iCurr);
				CqLath* pStart = pVisited;
				do
				{
					if(pVisited == pNew)	fValid=TqFalse;
					pVisited = pVisited->cf();
				}while(pVisited != pStart);
			}
			if(fValid)	Result.push_back(pNew);
		}
	}
}


//------------------------------------------------------------------------------
/**
 *	Get the number of vertices surrounding a facet.
 *	Get a count of laths representing the vertices which make up the facet this
 *	lath represents.
 *
 *	@return	Count of laths.
 */
TqInt CqLath::cQfv() const
{
	// Laths representing the edges of the associated facet are obtained by following
	// clockwise links around the face.
	TqInt c = 1;	// Start with this one.

	CqLath* pNext = cf();
	while(this != pNext)
	{
		assert(NULL != pNext);
		c++;
		pNext = pNext->cf();
	}
	return( c );
}


//------------------------------------------------------------------------------
/**
 *	Get the number of vertices emanating from this vertex.
 *	Get a count of laths representing the vertices emanating from the vertex
 *	this lath represents.
 *
 *	@return	Count of laths.
 */
TqInt CqLath::cQvv() const
{
	TqInt c = 1; // Start with this
	// Laths representing the edges that radiate from the associated vertex are obtained by 
	// following the clockwise vertex links around the vertex. 
	CqLath* pNext = cv();
	const CqLath* pLast = this;
	while(NULL != pNext && this != pNext)
	{
		c++;
		pLast = pNext;
		pNext = pNext->cv();
	}

	// If we hit a boundary, add the ec of this boundary edge and start again going backwards.
	// @warning Adding ccf for the boundary edge means that the lath represents a different vertex.
	if(NULL == pNext)
	{
		pLast = this;
		pNext = ccv();
		// We know we are going to hit a boundary in this direction as well so we can just look for that
		// case as a terminator.
		while(NULL != pNext)
		{
			assert( pNext != this );
			c++;
			pLast = pNext;
			pNext = pNext->ccv();
		}
		// We have hit the boundary going the other way, so add the ccf of this boundary edge.
		c++;
	}
	return( c );
}



END_NAMESPACE( Aqsis )
