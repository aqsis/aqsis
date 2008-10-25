//------------------------------------------------------------------------------
/**
 *	@file	trimcurve.cpp
 *	@author	Paul Gregory
 *	@brief	Implementation of trimcurce functionality.
 *
 *	Last change by:		$Author$
 *	Last change date:	$Date$
 */
//------------------------------------------------------------------------------


#include	"trimcurve.h"
#include	"surface.h"

namespace Aqsis {


//---------------------------------------------------------------------
/** Find the span in the knot vector containing the specified parameter value.
 */

TqUint CqTrimCurve::FindSpan( TqFloat u ) const
{
	if ( u >= m_aKnots[ m_cVerts ] )
		return ( m_cVerts -1 );
	if ( u <= m_aKnots[ Degree() ] )
		return ( Degree() );

	TqUint low = 0;
	TqUint high = m_cVerts + 1;
	TqUint mid = ( low + high ) / 2;

	while ( u < m_aKnots[ mid ] || u >= m_aKnots[ mid + 1 ] )
	{
		if ( u < m_aKnots[ mid ] )
			high = mid;
		else
			low = mid;
		mid = ( low + high ) / 2;
	}
	return ( mid );
}


//---------------------------------------------------------------------
/** Return the basis functions for the specified parameter value.
 */

void CqTrimCurve::BasisFunctions( TqFloat u, TqUint span, std::vector<TqFloat>& BasisVals )
{
	register TqInt r, s, i;
	register double omega;

	BasisVals[ 0 ] = 1.0;
	for ( r = 2; r <= static_cast<TqInt>( m_Order ); r++ )
	{
		i = span - r + 1;
		BasisVals[ r - 1 ] = 0.0;
		for ( s = r - 2; s >= 0; s-- )
		{
			i++;
			if ( i < 0 )
				omega = 0;
			else
				omega = ( u - m_aKnots[ i ] ) / ( m_aKnots[ i + r - 1 ] - m_aKnots[ i ] );

			BasisVals[ s + 1 ] = BasisVals[ s + 1 ] + ( 1 - omega ) * BasisVals[ s ];
			BasisVals[ s ] = omega * BasisVals[ s ];
		}
	}
}


//---------------------------------------------------------------------
/** Evaluate the nurbs curve at parameter value u.
 */

CqVector2D	CqTrimCurve::Evaluate( TqFloat u )
{
	std::vector<TqFloat> basis( m_Order );

	CqVector3D r( 0, 0, 0 );

	/* Evaluate non-uniform basis functions (and derivatives) */

	TqUint span = FindSpan( u );
	TqUint first = span - m_Order + 1;
	BasisFunctions( u, span, basis );

	// Weight control points against the basis functions
	TqUint j;
	for ( j = 0; j < m_Order; j++ )
	{
		TqUint rj = m_Order - 1L - j;

		TqFloat tmp = basis[ rj ];
		CqVector3D& cp = CP( j + first );

		r.x( r.x() + cp.x() * tmp );
		r.y( r.y() + cp.y() * tmp );
		r.z( r.z() + cp.z() * tmp );
	}

	return ( CqVector2D( r.x() / r.z(), r.y() / r.z() ) );
}



void CqTrimLoop::Prepare( CqSurface* pSurface )
{
	std::vector<CqTrimCurve>::iterator iCurve;
	std::vector<CqTrimCurve>::iterator iEnd = m_aCurves.end();
	TqInt iPoint;

	for ( iCurve = m_aCurves.begin(); iCurve != iEnd; iCurve++ )
	{
		TqInt cPoints = pSurface->TrimDecimation( *iCurve );

		iCurve->Clamp();

		TqFloat range = iCurve->aKnots() [ iCurve->cKnots() - 1 ] - iCurve->aKnots() [ 0 ];
		TqFloat u = iCurve->aKnots() [ 0 ];
		TqFloat du = range / cPoints;

		for ( iPoint = 0; iPoint < cPoints; iPoint++ )
		{
			// CqVector2D v(iCurve->Evaluate(u));
			// std::cout << v.x() << "," << v.y() << std::endl;
			m_aCurvePoints.push_back( iCurve->Evaluate( u ) );
			u += du;
		}
	}
}

/** This works by checking line segements in turn, and finding the ones
    that span the sample point in y. For those line segments, a test is 
	made to see which side of the line segment the point lies, and on which side
	of the sample point the intersection with the line segment is.
	For each crossing on the same side of the sample point, a state flag is flipped.
	If the state is inside at the end, then the sample is inside the polygon.
	See http://www.alienryderflex.com/polygon/
 */

const TqInt	CqTrimLoop::TrimPoint( const CqVector2D& v ) const
{
	TqFloat x = v.x();
	TqFloat y = v.y();
	TqInt i, j;
	bool oddNodes = false;
	TqInt size = m_aCurvePoints.size();
	for ( i = 0, j = size - 1; i < size; j = i++ )
	{
		TqFloat ax = m_aCurvePoints[ i ].x();
		TqFloat ay = m_aCurvePoints[ i ].y();
		TqFloat bx = m_aCurvePoints[ j ].x();
		TqFloat by = m_aCurvePoints[ j ].y();
		// Does this line segment span the point in y?
		if ( ( ( ( ay < y ) && ( by >= y ) ) ||
		        ( ( by < y ) && ( ay >= y ) ) ) )
		{
			// Does the horizontal intersection lie on the right side of the point?
			if( ax + ( y - ay ) / ( by - ay ) * ( bx - ax ) < x )
				// Then flip the state.
				oddNodes = ! oddNodes;
		}
	}
	return ( oddNodes );
}


const bool CqTrimLoop::LineIntersects(const CqVector2D& v1, const CqVector2D& v2) const
{
	TqFloat x1 = v1.x();
	TqFloat y1 = v1.y();
	TqFloat x2 = v2.x();
	TqFloat y2 = v2.y();

	TqInt i, j;
	TqInt size = m_aCurvePoints.size();
	for ( i = 0, j = size - 1; i < size; j = i++ )
	{
		TqFloat x3 = m_aCurvePoints[ i ].x();
		TqFloat y3 = m_aCurvePoints[ i ].y();
		TqFloat x4 = m_aCurvePoints[ j ].x();
		TqFloat y4 = m_aCurvePoints[ j ].y();

		TqFloat d = (x2-x1)*(y4-y3) - (y2-y1)*(x4-x3);
		if(d == 0.0f)
			continue;

		TqFloat r = ((y1-y3)*(x4-x3) - (x1-x3)*(y4-y3)) / d;
		TqFloat s = ((y1-y3)*(x2-x1) - (x1-x3)*(y2-y1)) / d;
		if( (r >= 0.0f) && (s >= 0.0f) && (r <= 1.0f) && (s <= 1.0f))
			return(true);
	}
	return(false);
}



void CqTrimLoopArray::Prepare( CqSurface* pSurface )
{
	std::vector<CqTrimLoop>::iterator iLoop;
	std::vector<CqTrimLoop>::iterator iEnd = m_aLoops.end();
	for ( iLoop = m_aLoops.begin(); iLoop != iEnd; iLoop++ )
		iLoop->Prepare( pSurface );
}


const bool	CqTrimLoopArray::TrimPoint( const CqVector2D& v ) const
{
	// Early out if no trim loops at all.
	if ( m_aLoops.size() == 0 )
		return ( false );

	TqInt	cCrosses = 0;

	std::vector<CqTrimLoop>::const_iterator iLoop;
	std::vector<CqTrimLoop>::const_iterator iEnd = m_aLoops.end();
	for ( iLoop = m_aLoops.begin(); iLoop != iEnd; iLoop++ )
		cCrosses += iLoop->TrimPoint( v )? 1 : 0;

	return ( !( cCrosses & 1 ) );
}


const bool	CqTrimLoopArray::LineIntersects( const CqVector2D& v1, const CqVector2D& v2 ) const
{
	// Early out if no trim loops at all.
	if ( m_aLoops.size() == 0 )
		return ( false );

	std::vector<CqTrimLoop>::const_iterator iLoop;
	std::vector<CqTrimLoop>::const_iterator iEnd = m_aLoops.end();
	for ( iLoop = m_aLoops.begin(); iLoop != iEnd; iLoop++ )
		if(iLoop->LineIntersects( v1, v2 ))
			return(true);

	return ( false );
}



//---------------------------------------------------------------------
/** Insert the specified knot into the U knot vector, and refine the control points accordingly.
 * \return The number of new knots created.
 */

TqUint CqTrimCurve::InsertKnot( TqFloat u, TqInt r )
{
	// Work on a copy.
	CqTrimCurve nS( *this );

	// Compute k and s      u = [ u_k , u_k+1)  with u_k having multiplicity s
	TqInt k = m_aKnots.size() - 1, s = 0;
	TqInt i, j;
	TqInt p = Degree();

	if ( u < m_aKnots[ Degree() ] || u > m_aKnots[ m_cVerts ] )
		return ( 0 );

	TqInt size = static_cast<TqInt>( m_aKnots.size() );
	for ( i = 0; i < size; i++ )
	{
		if ( m_aKnots[ i ] > u )
		{
			k = i - 1;
			break;
		}
	}

	if ( u <= m_aKnots[ k ] )
	{
		s = 1;
		for ( i = k; i > 0; i-- )
		{
			if ( m_aKnots[ i ] <= m_aKnots[ i - 1 ] )
				s++;
			else
				break;
		}
	}
	else
		s = 0;

	if ( ( r + s ) > p + 1 )
		r = p + 1 - s;

	if ( r <= 0 )
		return ( 0 );

	nS.Init( m_Order, m_cVerts + r );

	// Load new knot vector
	for ( i = 0;i <= k;i++ )
		nS.m_aKnots[ i ] = m_aKnots[ i ];
	for ( i = 1;i <= r;i++ )
		nS.m_aKnots[ k + i ] = u;
	size = static_cast<TqInt>( m_aKnots.size() );
	for ( i = k + 1;i < size; i++ )
		nS.m_aKnots[ i + r ] = m_aKnots[ i ];

	// Save unaltered control points
	std::vector<CqVector3D> R( p + 1 );

	// Insert control points as required on each row.
	for ( i = 0; i <= k - p; i++ )
		nS.CP( i ) = CP( i );
	size = static_cast<TqInt>( m_cVerts );
	for ( i = k - s; i < size; i++ )
		nS.CP( i + r ) = CP( i );
	for ( i = 0; i <= p - s; i++ )
		R[ i ] = CP( k - p + i );

	// Insert the knot r times
	TqUint L = 0 ;
	TqFloat alpha;
	for ( j = 1; j <= r; j++ )
	{
		L = k - p + j;
		TqInt i1;
		TqInt limit = p - j - s;
		for ( i = 0;i <= limit;i++ )
		{
			i1 = i + 1;
			alpha = ( u - m_aKnots[ L + i ] ) / ( m_aKnots[ k + i1 ] - m_aKnots[ L + i ] );
			R[ i ] = CqVector3D( alpha * R[ i1 ].x() + ( 1.0 - alpha ) * R[ i ].x(),
			                     alpha * R[ i1 ].y() + ( 1.0 - alpha ) * R[ i ].y(),
			                     alpha * R[ i1 ].z() + ( 1.0 - alpha ) * R[ i ].z() );
		}
		nS.CP( L ) = R[ 0 ];
		if ( p - j - s > 0 )
			nS.CP( k + r - j - s ) = R[ p - j - s ];
	}

	// Load remaining control points
	for ( i = L + 1; i < k - s; i++ )
		nS.CP( i ) = R[ i - L ];

	*this = nS;

	return ( r );
}



//---------------------------------------------------------------------
/** Ensure a nonperiodic (clamped) knot vector by inserting U[p] and U[m-p] multiple times.
 */

void CqTrimCurve::Clamp()
{
	TqUint n1 = InsertKnot( m_aKnots[ Degree() ], Degree() );
	TqUint n2 = InsertKnot( m_aKnots[ m_cVerts ], Degree() );

	// Now trim unnecessary knots and control points
	if ( n1 || n2 )
	{
		CqTrimCurve nS( *this );
		m_aKnots.resize( m_aKnots.size() - n1 - n2 );
		m_aVerts.resize( m_cVerts - n1 - n2 );
		m_cVerts -= n1 + n2;
		TqUint i;
		TqInt size = nS.m_aKnots.size() - n2;
		for ( i = n1; i < static_cast<TqUint>( size ); i++ )
			m_aKnots[ i - n1 ] = nS.m_aKnots[ i ];
		size = nS.m_cVerts - n2;
		for ( i = n1; i < static_cast<TqUint>( size ); i++ )
			CP( i - n1 ) = nS.CP( i );
	}
}


//---------------------------------------------------------------------

} // namespace Aqsis
