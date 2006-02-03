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
#include	"pool.h"
#include	<vector>

START_NAMESPACE( Aqsis )


//------------------------------------------------------------------------------
/**
 *	Individual topology element class.
 *	Holds information about mesh neighbourhoods allowing easy data aextraction about mesh topology.
 */

class CqLath
{
	public:
		///	Constructor.
		CqLath()	 : m_pClockwiseVertex(NULL), m_pClockwiseFacet(NULL),  m_pParentFacet(NULL), m_pChildVertex(NULL), m_pMidVertex(NULL), m_pFaceVertex(NULL), m_VertexIndex(0), m_FaceVertexIndex(0)
		{}
		CqLath( TqInt iV, TqInt iFV ) : m_pClockwiseVertex(NULL), m_pClockwiseFacet(NULL),m_pParentFacet(NULL), m_pChildVertex(NULL), m_pMidVertex(NULL), m_pFaceVertex(NULL), m_VertexIndex( iV ), m_FaceVertexIndex( iFV )
		{}

		///	Destructor.
		~CqLath()
		{}

		/** Overridden operator new to allocate micropolys from a pool.
		 */
		void* operator new( size_t size )
		{
			return( m_thePool.alloc() );
		}

		/** Overridden operator delete to allocate micropolys from a pool.
		 */
		void operator delete( void* p )
		{
			m_thePool.free( p );
		}

		/// Get a pointer to the lath representing the facet that this one was created from.
		CqLath*	pParentFacet() const
		{
			return(m_pParentFacet);
		}
		/// Get a pointer to the lath representing this vertex at the next level.
		CqLath*	pChildVertex() const
		{
			return(m_pChildVertex);
		}
		/// Get a pointer to the lath representing the midpoint vertex of this edge at the next level.
		CqLath*	pMidVertex() const
		{
			return(m_pMidVertex);
		}
		/// Get a pointer to the lath representing the midpoint vertex of this face at the next level.
		CqLath*	pFaceVertex() const
		{
			return(m_pFaceVertex);
		}

		/// Get the index of the vertex this lath references.
		TqInt	VertexIndex() const
		{
			return(m_VertexIndex);
		}
		/// Get the index of the vertex this lath references.
		TqInt	FaceVertexIndex() const
		{
			return(m_FaceVertexIndex);
		}

		/// Set the pointer to the next lath clockwise about the vertex.
		void		SetpClockwiseVertex(CqLath* pLath)
		{
			m_pClockwiseVertex=pLath;
		}
		/// Set the pointer to the next lath clockwise about the facet.
		void		SetpClockwiseFacet(CqLath* pLath)
		{
			m_pClockwiseFacet=pLath;
		}
		/// Set the pointer to the lath representing the facet that this one was created from.
		void		SetpParentFacet(CqLath* pLath)
		{
			m_pParentFacet=pLath;
		}
		/// Set the pointer to the lath representing this vertex at the next level.
		void		SetpChildVertex(CqLath* pLath)
		{
			m_pChildVertex=pLath;
		}
		/// Set the pointer to the lath representing the midpoint vertex of this edge at the next level.
		void		SetpMidVertex(CqLath* pLath)
		{
			m_pMidVertex=pLath;
		}
		/// Set the pointer to the lath representing the midpoint vertex of this face at the next level.
		void		SetpFaceVertex(CqLath* pLath)
		{
			m_pFaceVertex=pLath;
		}

		/// Set the index of the vertex this lath refers to.
		void		SetVertexIndex(TqInt iV)
		{
			m_VertexIndex=iV;
		}
		/// Set the index of the face vertex this lath refers to.
		void		SetFaceVertexIndex(TqInt iV)
		{
			m_FaceVertexIndex=iV;
		}

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
		void Qef(std::vector<const CqLath*>&) const;
		void Qev(std::vector<const CqLath*>&) const;
		void Qfe(std::vector<const CqLath*>&) const;
		void Qve(std::vector<const CqLath*>&) const;
		void Qfv(std::vector<const CqLath*>&) const;
		void Qvv(std::vector<const CqLath*>&) const;
		void Qvf(std::vector<const CqLath*>&) const;
		void Qee(std::vector<const CqLath*>&) const;
		void Qff(std::vector<const CqLath*>&) const;

		TqInt cQfv() const;
		TqInt cQvv() const;
		TqInt cQve() const;
		TqInt cQvf() const;

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

		CqLath* ccfBoundary() const;

		CqLath*	m_pClockwiseVertex;
		CqLath*	m_pClockwiseFacet;

		// Hierarchical subdivision data
		CqLath* m_pParentFacet;		///< Pointer to the facet that was subdivided to produce this one.
		CqLath* m_pChildVertex;		///< Pointer to the child point that represents this point at the next level.
		CqLath*	m_pMidVertex;		///< Pointer to the point that represents the midpoint of this edge at the next level.
		CqLath*	m_pFaceVertex;		///< Pointer to the point that represents the midpoint of this face at the next level.

		TqInt	m_VertexIndex;
		TqInt	m_FaceVertexIndex;

		static	CqObjectPool<CqLath>	m_thePool;
};


//------------------------------------------------------------------------------
/**
 *	Get the next lath clockwise around the facet.
 *	Get a pointer to the next lath in a clockwise direction around the
 *	associated facet. This information is inherent in the data structures.
 *
 *	@return	Pointer to the lath.
 */
inline CqLath* CqLath::cf() const
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
inline CqLath* CqLath::cv() const
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
inline CqLath* CqLath::ec() const
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
 *	Get the lath counter clockwise about the vertex.
 *	Get a pointer to the next lath in a counter clockwise direction about the
 *	associated vertex. This function is constant in all cases.
 *
 *	@return	Pointer to the lath.
 */
inline CqLath* CqLath::ccv() const
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
 *	Get the lath counter clockwise about the facet.
 *	Get a pointer to the next lath in a counter clockwise direction about the
 *	associated facet. This function is constant in all cases excepth where the
 *	associated edge is a boundary edge, in which case it is linear in the
 *	number of edges in the associated facet.
 *
 *	@return	Pointer to the lath.
 */
inline CqLath* CqLath::ccf() const
{
	// If the associated edge is boundary, we will need to search backwards.
	if(NULL != ec() && NULL != ec()->cv())
		return(ec()->cv());
	else
		return ccfBoundary();
}


//------------------------------------------------------------------------------
/**
 *	Get the faces surrounding an edge.
 *	Get a list of laths representing the faces surrounding an edge, will
 *	return just one if the edge is a boundary.
 *
 *	@return	Pointer to an array of lath pointers.
 */
inline void CqLath::Qef(std::vector<CqLath*>& Result)
{
	Result.resize(NULL != ec()? 2 : 1);
	// Laths representing the two faces bounding an edge are given by L and L->ec(). If edge
	// is a boundary, only L is passed back.
	CqLath *pTmpLath = this;
	Result[0] = pTmpLath;

	if(NULL != ec())
		Result[1] = ec();
}


//------------------------------------------------------------------------------
/**
 *	Get the vertices surounding an edge.
 *	Get a list of laths representing the vertices making up an edge.
 *
 *	@return	Pointer to an array of lath pointers.
 */
inline void CqLath::Qev(std::vector<CqLath*>& Result)
{
	Result.resize(2);
	// Laths representing the two vertices of the associated edge are given by
	// L and L->ccf(). Note we use cf here because itis guarunteed, whereas ec is not.
	CqLath *pTmpLath = this;
	Result[0] = pTmpLath;
	Result[1] = ccf();
}


//------------------------------------------------------------------------------
/**
 *	Get the vertices surrounding a facet.
 *	Get a list of laths representing the vertices which make up the facet this
 *	lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
inline void CqLath::Qfv(std::vector<CqLath*>& Result)
{
	Qfe(Result);
}


//------------------------------------------------------------------------------
/**
 *	Get the faces surrounding an edge.
 *	Get a list of laths representing the faces surrounding an edge, will
 *	return just one if the edge is a boundary.
 *
 *	@return	Pointer to an array of lath pointers.
 */
inline void CqLath::Qef(std::vector<const CqLath*>& Result) const
{
	Result.resize(NULL != ec()? 2 : 1);
	// Laths representing the two faces bounding an edge are given by L and L->ec(). If edge
	// is a boundary, only L is passed back.
	const CqLath *pTmpLath = this;
	Result[0] = pTmpLath;

	if(NULL != ec())
		Result[1] = ec();
}


//------------------------------------------------------------------------------
/**
 *	Get the vertices surounding an edge.
 *	Get a list of laths representing the vertices making up an edge.
 *
 *	@return	Pointer to an array of lath pointers.
 */
inline void CqLath::Qev(std::vector<const CqLath*>& Result) const
{
	Result.resize(2);
	// Laths representing the two vertices of the associated edge are given by
	// L and L->ccf(). Note we use cf here because itis guarunteed, whereas ec is not.
	const CqLath *pTmpLath = this;
	Result[0] = pTmpLath;
	Result[1] = ccf();
}


//------------------------------------------------------------------------------
/**
 *	Get the vertices surrounding a facet.
 *	Get a list of laths representing the vertices which make up the facet this
 *	lath represents.
 *
 *	@return	Pointer to an array of lath pointers.
 */
inline void CqLath::Qfv(std::vector<const CqLath*>& Result) const
{
	Qfe(Result);
}

END_NAMESPACE( Aqsis )

#endif	//	LATH_H_LOADED
