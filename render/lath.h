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
		\brief Declares the classes for storing mesh topology information.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#ifndef	LATH_H_LOADED
#define	LATH_H_LOADED

#include	"aqsis.h"
#include	"memorypool.h"
#include	<vector>

START_NAMESPACE( Aqsis )


//------------------------------------------------------------------------------
/**
 *	Individual topology element class.
 *	Holds information about mesh neighbourhoods allowing easy data aextraction about mesh topology.
 */

class CqLath : public CqPoolable<CqLath, 512>
{
public:
	///	Constructor.
	CqLath()	 : m_pClockwiseVertex(NULL), m_pClockwiseFacet(NULL),  m_pParentFacet(NULL), m_pChildVertex(NULL), m_pMidVertex(NULL), m_pFaceVertex(NULL), m_VertexIndex(0), m_FaceVertexIndex(0)
				   {}
	CqLath( TqInt iV, TqInt iFV ) : m_pClockwiseVertex(NULL), m_pClockwiseFacet(NULL),m_pParentFacet(NULL), m_pChildVertex(NULL), m_pMidVertex(NULL), m_pFaceVertex(NULL), m_VertexIndex( iV ), m_FaceVertexIndex( iFV )
				   {}

	///	Destructor.
	~CqLath()	{}

	/// Get a pointer to the lath representing the facet that this one was created from.
	CqLath*	pParentFacet() const		{return(m_pParentFacet);}
	/// Get a pointer to the lath representing this vertex at the next level.
	CqLath*	pChildVertex() const		{return(m_pChildVertex);}
	/// Get a pointer to the lath representing the midpoint vertex of this edge at the next level.
	CqLath*	pMidVertex() const			{return(m_pMidVertex);}
	/// Get a pointer to the lath representing the midpoint vertex of this face at the next level.
	CqLath*	pFaceVertex() const			{return(m_pFaceVertex);}

	/// Get the index of the vertex this lath references.
	TqInt	VertexIndex() const	{return(m_VertexIndex);}
	/// Get the index of the vertex this lath references.
	TqInt	FaceVertexIndex() const	{return(m_FaceVertexIndex);}

	/// Set the pointer to the next lath clockwise about the vertex.
	void		SetpClockwiseVertex(CqLath* pLath)		
											{m_pClockwiseVertex=pLath;}
	/// Set the pointer to the next lath clockwise about the facet.
	void		SetpClockwiseFacet(CqLath* pLath)		
											{m_pClockwiseFacet=pLath;}
	/// Set the pointer to the lath representing the facet that this one was created from.
	void		SetpParentFacet(CqLath* pLath)		
											{m_pParentFacet=pLath;}
	/// Set the pointer to the lath representing this vertex at the next level.
	void		SetpChildVertex(CqLath* pLath)		
											{m_pChildVertex=pLath;}
	/// Set the pointer to the lath representing the midpoint vertex of this edge at the next level.
	void		SetpMidVertex(CqLath* pLath)	{m_pMidVertex=pLath;}
	/// Set the pointer to the lath representing the midpoint vertex of this face at the next level.
	void		SetpFaceVertex(CqLath* pLath)	{m_pFaceVertex=pLath;}

	/// Set the index of the vertex this lath refers to.
	void		SetVertexIndex(TqInt iV)	{m_VertexIndex=iV;}
	/// Set the index of the face vertex this lath refers to.
	void		SetFaceVertexIndex(TqInt iV)	{m_FaceVertexIndex=iV;}

	// Basic neighbourhood operators.

	CqLath*	cf() const;
	CqLath*	cv() const;
	CqLath*	ec() const;
	CqLath*	ccf() const;
	CqLath*	ccv() const;

	// Data access primitives
	
	void Qef(std::vector<CqLath*>&);
	void Qev(std::vector<CqLath*>&);
	void Qfe(std::vector<CqLath*>&);
	void Qve(std::vector<CqLath*>&);
	void Qfv(std::vector<CqLath*>&);
	void Qvv(std::vector<CqLath*>&);
	void Qvf(std::vector<CqLath*>&);
	void Qee(std::vector<CqLath*>&);
	void Qff(std::vector<CqLath*>&);

	TqInt cQfv() const;
	TqInt cQvv() const;

	TqBool isBoundaryFacet()
	{
		// Check if any of the vertices are boundary, if so then this facet must be.
		std::vector<CqLath*> aQfv;
		Qfv(aQfv);
		std::vector<CqLath*>::iterator iVert;
		for( iVert = aQfv.begin(); iVert != aQfv.end(); iVert++ )
			if( (*iVert)->isBoundaryVertex() )
				return( TqTrue );
		return( TqFalse );
	}

	TqBool isBoundaryEdge()
	{
		// If this edge has no companion it must be boundary.
		if( NULL == ec() )
			return( TqTrue );
		return( TqFalse );
	}

	TqBool isBoundaryVertex()
	{
		// Check if the ccv loop is closed, if not must be boundary.
		CqLath* pNext = ccv();
		while( pNext != this )
		{
			if( NULL == pNext )
				return( TqTrue );
			pNext = pNext->ccv();
		}
		return( TqFalse );
	}

private:
	///	Declared private to prevent copying.
	CqLath(const CqLath &);
	///	Declared private to prevent copying.
	CqLath &	operator=(const CqLath &);

	CqLath*	m_pClockwiseVertex;
	CqLath*	m_pClockwiseFacet;

	// Hierarchical subdivision data
	CqLath* m_pParentFacet;		///< Pointer to the facet that was subdivided to produce this one.
	CqLath* m_pChildVertex;		///< Pointer to the child point that represents this point at the next level.
	CqLath*	m_pMidVertex;		///< Pointer to the point that represents the midpoint of this edge at the next level.
	CqLath*	m_pFaceVertex;		///< Pointer to the point that represents the midpoint of this face at the next level.

	TqInt	m_VertexIndex;
	TqInt	m_FaceVertexIndex;
};

END_NAMESPACE( Aqsis )

#endif	//	LATH_H_LOADED
