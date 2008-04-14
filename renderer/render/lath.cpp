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
		\brief Implements classes for storing mesh topology information..
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include	"lath.h"


namespace Aqsis {

CqObjectPool<CqLath>	CqLath::m_thePool;

//------------------------------------------------------------------------------
/**
 *	Get the lath counter clockwise about the facet.
 *	Get a pointer to the next lath in a counter clockwise direction about the
 *	associated facet, where the associated edge is a boundary edge.
 *	@return	Pointer to the lath.
 */
CqLath* CqLath::ccfBoundary() const
{
	// The associated edge is boundary, we will need to search backwards.
	CqLath* pLdash=cf();
	while(1)
	{
		CqLath* temp = pLdash->cf();
		if(this == temp || NULL == temp)
			break;
		pLdash=temp;
	}
	//        while(this != pLdash->cf() && NULL != pLdash->cf())
	//            pLdash=pLdash->cf();
	assert(this == pLdash->cf());
	return(pLdash);
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
	TqInt len = 1;
	CqLath* pNext = cf();
	CqLath* pNexta = pNext;
	while(this != pNext)
	{
		assert(NULL != pNext);
		len++;
		pNext = pNext->cf();
	}

	Result.resize(len);
	// Laths representing the edges of the associated facet are obtained by following
	// clockwise links around the face.
	CqLath *pTmpLath = this;
	Result[0] = pTmpLath;

	TqInt index = 1;
	while(this != pNexta)
	{
		Result[index++] = pNexta;
		pNexta = pNexta->cf();
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
	TqInt len = cQve();

	CqLath* pNext = cv();
	CqLath* pLast = this;

	Result.resize(len);
	TqInt index = 0;
	// Laths representing the edges that radiate from the associated vertex are obtained by
	// following the clockwise vertex links around the vertex.
	CqLath *pTmpLath = this;
	Result[index++] = pTmpLath;

	while(NULL != pNext && this != pNext)
	{
		Result[index++] = pNext;
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
			Result[index++] = pNext;
			pLast = pNext;
			pNext = pNext->ccv();
		}
		// We have hit the boundary going the other way, so add the ccf of this boundary edge.
		Result[index++] = pLast->cf();
	}
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
	TqInt len = cQvf();

	CqLath* pNext = cv();

	Result.resize(len);
	TqInt index = 0;

	// Laths representing the edges that radiate from the associated vertex are obtained by
	// following the clockwise vertex links around the vertex.
	CqLath *pTmpLath = this;
	Result[index++] = pTmpLath;

	while(NULL != pNext && this != pNext)
	{
		Result[index++] = pNext;
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
			Result[index++] = pNext;
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
	Result.swap(ResQve1);
	//Result.insert(Result.end(), ResQve1.begin(), ResQve1.end());

	std::vector<CqLath*>::iterator iLath;
	TqInt len2 = 0;
	for(iLath = ResQve2.begin(); iLath!=ResQve2.end(); iLath++)
	{
		if(ec() != (*iLath) && this != (*iLath))
			len2++;
	}

	TqInt index = Result.size();
	Result.resize( Result.size() + len2 );
	for(iLath = ResQve2.begin(); iLath!=ResQve2.end(); iLath++)
	{
		if(ec() != (*iLath) && this != (*iLath))
			Result[index++] = (*iLath);
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
	std::vector<CqLath*> ResQfe;
	Qfe(ResQfe);

	// The laths representing the edges radiating from the two vertices of the edge this lath represents
	// can be implemented by taking the union of Qve for this and cf() and removing the duplicate ec() if
	// it exists.
	std::vector<CqLath*>::iterator iLath;
	TqInt len = 0;
	for(iLath = ResQfe.begin(); iLath!=ResQfe.end(); iLath++)
		len += (*iLath)->cQve();

	Result.resize(0);
	Result.reserve(len);

	for(iLath = ResQfe.begin(); iLath!=ResQfe.end(); ++iLath)
	{
		std::vector<CqLath*> ResQev;
		(*iLath)->Qve(ResQev);
		std::vector<CqLath*>::iterator iEdge;
		for(iEdge = ResQev.begin(); iEdge!=ResQev.end(); ++iEdge)
		{
			CqLath* pNew = (*iEdge);
			// Search for the new candidate by traversing the cf lists for all laths currrently in the
			// result list.
			bool fValid = true;
			std::vector<CqLath*>::iterator iCurr;
			for(iCurr = Result.begin(); iCurr!=Result.end() && fValid; ++iCurr)
			{
				CqLath* pVisited =(*iCurr);
				CqLath* pStart = pVisited;
				do
				{
					if(pVisited == pNew)
					{
						fValid=false;
						break;
					}
					pVisited = pVisited->cf();
				}
				while(pVisited != pStart);
			}
			if(fValid)
				Result.push_back(pNew);
		}
	}
}



//------------------------------------------------------------------------------
/**
 *	Get the edges surrounding a facet.
 *	Get a list of laths representing the esges making up a facet.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qfe(std::vector<const CqLath*>& Result) const
{
	TqInt len = 1;
	const CqLath* pNext = cf();
	const CqLath* pNexta = pNext;
	while(this != pNext)
	{
		assert(NULL != pNext);
		len++;
		pNext = pNext->cf();
	}

	Result.resize(len);
	// Laths representing the edges of the associated facet are obtained by following
	// clockwise links around the face.
	const CqLath *pTmpLath = this;
	Result[0] = pTmpLath;

	TqInt index = 1;
	while(this != pNexta)
	{
		Result[index++] = pNexta;
		pNexta = pNexta->cf();
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
void CqLath::Qve(std::vector<const CqLath*>& Result) const
{
	TqInt len = cQve();

	const CqLath* pNext = cv();
	const CqLath* pLast = this;

	Result.resize(len);
	TqInt index = 0;
	// Laths representing the edges that radiate from the associated vertex are obtained by
	// following the clockwise vertex links around the vertex.
	const CqLath *pTmpLath = this;
	Result[index++] = pTmpLath;

	while(NULL != pNext && this != pNext)
	{
		Result[index++] = pNext;
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
			Result[index++] = pNext;
			pLast = pNext;
			pNext = pNext->ccv();
		}
		// We have hit the boundary going the other way, so add the ccf of this boundary edge.
		Result[index++] = pLast->cf();
	}
}



//------------------------------------------------------------------------------
/**
 *	Get the vertices emanating from this vertex.
 *	Get a list of laths representing the vertices emanating from the vertex
 *	this lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
void CqLath::Qvv(std::vector<const CqLath*>& Result) const 
{
	Qve(Result);

	// We can get the laths for the vertices surrounding a vertex by getting the cf() for each
	// lath in Qev. Note we must check first if the lath in Qve represents the same vertex as this
	// as if there is a boundary case, the lath on the clockwise boundary will already point to the
	// opposite vertex.
	for(std::vector<const CqLath*>::iterator iLath = Result.begin(); iLath!=Result.end(); iLath++)
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
void CqLath::Qvf(std::vector<const CqLath*>& Result) const
{
	TqInt len = cQvf();

	const CqLath* pNext = cv();

	Result.resize(len);
	TqInt index = 0;

	// Laths representing the edges that radiate from the associated vertex are obtained by
	// following the clockwise vertex links around the vertex.
	const CqLath *pTmpLath = this;
	Result[index++] = pTmpLath;

	while(NULL != pNext && this != pNext)
	{
		Result[index++] = pNext;
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
			Result[index++] = pNext;
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
void CqLath::Qee(std::vector<const CqLath*>& Result) const
{
	Result.clear();
	std::vector<const CqLath*> ResQve1;
	Qve(ResQve1);
	std::vector<const CqLath*> ResQve2;
	ccf()->Qve(ResQve2);

	// The laths representing the edges radiating from the two vertices of the edge this lath represents
	// can be implemented by taking the union of Qve for this and cf() and removing the duplicate cf() if
	// it exists.
	Result.swap(ResQve1);
	//Result.insert(Result.end(), ResQve1.begin(), ResQve1.end());

	std::vector<const CqLath*>::iterator iLath;
	TqInt len2 = 0;
	for(iLath = ResQve2.begin(); iLath!=ResQve2.end(); iLath++)
	{
		if(ec() != (*iLath) && this != (*iLath))
			len2++;
	}

	TqInt index = Result.size();
	Result.resize( Result.size() + len2 );
	for(iLath = ResQve2.begin(); iLath!=ResQve2.end(); iLath++)
	{
		if(ec() != (*iLath) && this != (*iLath))
			Result[index++] = (*iLath);
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
void CqLath::Qff(std::vector<const CqLath*>& Result) const
{
	std::vector<const CqLath*> ResQfe;
	Qfe(ResQfe);

	// The laths representing the edges radiating from the two vertices of the edge this lath represents
	// can be implemented by taking the union of Qve for this and cf() and removing the duplicate ec() if
	// it exists.
	std::vector<const CqLath*>::iterator iLath;
	TqInt len = 0;
	for(iLath = ResQfe.begin(); iLath!=ResQfe.end(); iLath++)
		len += (*iLath)->cQve();

	Result.resize(0);
	Result.reserve(len);

	for(iLath = ResQfe.begin(); iLath!=ResQfe.end(); ++iLath)
	{
		std::vector<const CqLath*> ResQev;
		(*iLath)->Qve(ResQev);
		std::vector<const CqLath*>::iterator iEdge;
		for(iEdge = ResQev.begin(); iEdge!=ResQev.end(); ++iEdge)
		{
			const CqLath* pNew = (*iEdge);
			// Search for the new candidate by traversing the cf lists for all laths currrently in the
			// result list.
			bool fValid = true;
			std::vector<const CqLath*>::iterator iCurr;
			for(iCurr = Result.begin(); iCurr!=Result.end() && fValid; ++iCurr)
			{
				const CqLath* pVisited =(*iCurr);
				const CqLath* pStart = pVisited;
				do
				{
					if(pVisited == pNew)
					{
						fValid=false;
						break;
					}
					pVisited = pVisited->cf();
				}
				while(pVisited != pStart);
			}
			if(fValid)
				Result.push_back(pNew);
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


//------------------------------------------------------------------------------
/**
 *	Get the edges emanating from a vertex.
 *	Get a list of laths representing the edges which emanate from the vertex
 *	this lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
TqInt CqLath::cQve() const
{
	TqInt len = 1;

	CqLath* pNext = cv();
	const CqLath* pLast = this;
	while(NULL != pNext && this != pNext)
	{
		len++;
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
			len++;
			pLast = pNext;
			pNext = pNext->ccv();
		}
		// We have hit the boundary going the other way, so add the ccf of this boundary edge.
		len++;
	}
	return(len);
}


//------------------------------------------------------------------------------
/**
 *	Get the count of facets which share this vertex.
 *	Get a count of laths which represent the facets which share the vertex this
 *	lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
TqInt CqLath::cQvf() const
{
	TqInt len = 1;

	CqLath* pNext = cv();
	while(NULL != pNext && this != pNext)
	{
		len++;
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
			len++;
			pNext = pNext->ccv();
		}
	}
	return(len);
}


} // namespace Aqsis
