//------------------------------------------------------------------------------
/**
 *	@file	trimcurve.h
 *	@author	Paul Gregory
 *	@brief	NURB based trim cureve class.
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------

#ifndef TRIMCURVE_H_INCLUDED
#define TRIMCURVE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <vector>

#include <aqsis/math/vector2d.h>
#include <aqsis/math/vector3d.h>

namespace Aqsis {

class CqSurface;

//----------------------------------------------------------------------
/** \class CqTrimCurve
 * Trim curve, based on NURBS surface, with control to restrict to 2d.
 */

class CqTrimCurve
{
	private:
	public:
		CqTrimCurve()
		{}
		virtual	~CqTrimCurve()
		{}

		/** Get the order of the NURBS surface in the u direction.
		 */
		TqUint	Order() const
		{
			return ( m_Order );
		}
		/** Get the order of the NURBS surface in the v direction.
		 */
		TqUint	Degree() const
		{
			return ( m_Order -1 );
		}
		/** Get the number of control points in the u direction.
		 */
		TqUint	cVerts() const
		{
			return ( m_cVerts );
		}
		/** Get the length of the knot vector for the u direction.
		 */
		TqUint	cKnots() const
		{
			return ( m_cVerts + m_Order );
		}
		/** Get a reference to the knot vector for the u direction.
		 */
		std::vector<TqFloat>& aKnots()
		{
			return ( m_aKnots );
		}

		/** Get the control point at the specified u,v index.
		 * \param u Index in the u direction.
		 * \return Reference to the 4D homogenous control point.
		 */
		CqVector3D&	CP( const TqUint u )
		{
			return ( m_aVerts[ u ] );
		}
		/** Get the control point at the specified u,v index.
		 * \param u Index in the u direction.
		 * \return Reference to the 4D homogenous control point.
		 */
		const	CqVector3D&	CP( const TqUint u ) const
		{
			return ( m_aVerts[ u ] );
		}

		/** Initialise the NURBS structures to take a NURBS curve of the specified dimensions.
		 * \param Order The required order.
		 * \param cVerts The required control point count.
		 */
		void	Init( TqUint Order, TqUint cVerts )
		{
			TqUint cKnots = cVerts + Order;
			m_aKnots.resize( cKnots );
			m_aVerts.resize( cVerts );
			m_Order = Order;
			m_cVerts = cVerts;
		}

		TqUint	FindSpan( TqFloat u ) const;
		void	BasisFunctions( TqFloat u, TqUint span, std::vector<TqFloat>& BasisVals );
		CqVector2D	Evaluate( TqFloat u );
		TqUint	InsertKnot( TqFloat u, TqInt r );
		void	Clamp();

	protected:
		std::vector<TqFloat>	m_aKnots;	///< Knot vector.
		TqUint	m_Order;	///< Surface order.
		TqUint	m_cVerts;	///< Control point.
		std::vector<CqVector3D>	m_aVerts;	///< Nurbs control points.
}
;



class CqTrimLoop
{
	public:
		CqTrimLoop()
		{}
		~CqTrimLoop()
		{}


		std::vector<CqTrimCurve>& aCurves()
		{
			return ( m_aCurves );
		}

		void	Prepare( CqSurface* pSurface );
		const	TqInt	TrimPoint( const CqVector2D& v ) const;
		const	bool	LineIntersects(const CqVector2D& v1, const CqVector2D& v2) const;

	private:
		std::vector<CqTrimCurve>	m_aCurves;
		std::vector<CqVector2D>	m_aCurvePoints;
};


class CqTrimLoopArray
{
	public:
		CqTrimLoopArray()
		{}
		~CqTrimLoopArray()
		{}

		std::vector<CqTrimLoop>& aLoops()
		{
			return ( m_aLoops );
		}

		void	Prepare( CqSurface* pSurface );
		const	bool	TrimPoint( const CqVector2D& v ) const;
		const	bool	LineIntersects(const CqVector2D& v1, const CqVector2D& v2) const;
		void	Clear()
		{
			m_aLoops.resize( 0 );
		}

	private:
		std::vector<CqTrimLoop>	m_aLoops;
};


//-----------------------------------------------------------------------

} // namespace Aqsis


#endif // TRIMCURVE_H_INCLUDED
