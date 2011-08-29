// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
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
		\brief Implements the CqSurfaceNurbs classes for handling Renderman NURBS primitives.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>

#include	<math.h>
#include	<stdio.h>

#include	"nurbs.h"
#include	"renderer.h"
#include	<aqsis/math/vector3d.h>
#include	"bilinear.h"
#include	"attributes.h"
#include	<aqsis/math/math.h>

namespace Aqsis {

//---------------------------------------------------------------------
/** Constructor.
 */

CqSurfaceNURBS::CqSurfaceNURBS() : CqSurface(), m_uOrder( 0 ), m_vOrder( 0 ), m_cuVerts( 0 ), m_cvVerts( 0 ), m_umin( 0.0f ), m_umax( 1.0f ), m_vmin( 0.0f ), m_vmax( 1.0f ), m_fPatchMesh( false )
{
	TrimLoops() = static_cast<const CqAttributes*>(pAttributes().get())->TrimLoops();

	STATS_INC( GPR_nurbs );
}

//---------------------------------------------------------------------
/** Copy constructor.
 */

/* CqSurfaceNURBS::CqSurfaceNURBS( const CqSurfaceNURBS& From )
 * {
 * 	*this = From;
 * 
 * 	STATS_INC( GPR_nurbs );
 * }
 */


//---------------------------------------------------------------------
/** Create a clone of this NURBS surface.
 */

CqSurface* CqSurfaceNURBS::Clone() const
{
	CqSurfaceNURBS* clone = new CqSurfaceNURBS();

	CqSurface::CloneData( clone );

	// Initialise the NURBS surface.
	clone->Init( m_uOrder, m_vOrder, m_cuVerts, m_cvVerts );

	clone->m_umin = m_umin;
	clone->m_umax = m_umax;
	clone->m_vmin = m_vmin;
	clone->m_vmax = m_vmax;

	clone->m_fPatchMesh = m_fPatchMesh;

	// Copy the knot vectors.
	TqInt i;
	for ( i = m_auKnots.size() - 1; i >= 0; i-- )
		clone->m_auKnots[ i ] = m_auKnots[ i ];
	for ( i = m_avKnots.size() - 1; i >= 0; i-- )
		clone->m_avKnots[ i ] = m_avKnots[ i ];

	clone->TrimLoops() = TrimLoops();

	return(clone);
}



//---------------------------------------------------------------------
/** Comparison operator.
 */

TqInt CqSurfaceNURBS::operator==( const CqSurfaceNURBS& from )
{
	if ( from.m_cuVerts != m_cuVerts || from.m_cvVerts != m_cvVerts )
		return ( 0 );

	if ( from.m_uOrder != m_uOrder || from.m_vOrder != m_vOrder )
		return ( 0 );

	TqInt i;
	for ( i = P() ->Size() - 1; i >= 0; i-- )
	{
		if ( P()->pValue( i )[0] != from.P()->pValue( i )[0] )
			return ( 0 );
	}

	for ( i = m_auKnots.size() - 1; i >= 0; i-- )
	{
		if ( m_auKnots[ i ] != from.m_auKnots[ i ] )
			return ( 0 );
	}

	for ( i = m_avKnots.size() - 1; i >= 0; i-- )
	{
		if ( m_avKnots[ i ] != from.m_avKnots[ i ] )
			return ( 0 );
	}
	return ( 1 );
}


//---------------------------------------------------------------------
/** Find the span in the U knot vector containing the specified parameter value.
 */

TqUint CqSurfaceNURBS::FindSpanU( TqFloat u ) const
{
	if ( u >= m_auKnots[ m_cuVerts ] )
		return ( m_cuVerts -1 );
	if ( u <= m_auKnots[ uDegree() ] )
		return ( uDegree() );

	TqUint low = 0;
	TqUint high = m_cuVerts + 1;
	TqUint mid = ( low + high ) / 2;

	while ( u < m_auKnots[ mid ] || u >= m_auKnots[ mid + 1 ] )
	{
		if ( u < m_auKnots[ mid ] )
			high = mid;
		else
			low = mid;
		mid = ( low + high ) / 2;
	}
	return ( mid );
}


//---------------------------------------------------------------------
/** Find the span in the V knot vector containing the specified parameter value.
 */

TqUint CqSurfaceNURBS::FindSpanV( TqFloat v ) const
{
	if ( v >= m_avKnots[ m_cvVerts ] )
		return ( m_cvVerts -1 );
	if ( v <= m_avKnots[ vDegree() ] )
		return ( vDegree() );

	TqUint low = 0;
	TqUint high = m_cvVerts + 1;
	TqUint mid = ( low + high ) / 2;

	while ( v < m_avKnots[ mid ] || v >= m_avKnots[ mid + 1 ] )
	{
		if ( v < m_avKnots[ mid ] )
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

void CqSurfaceNURBS::BasisFunctions( TqFloat u, TqUint i, std::vector<TqFloat>& U, TqInt k, std::vector<TqFloat>& N )
{
	register TqInt j, r;
	register TqFloat saved, temp;
	std::vector<TqFloat> left( k ), right( k );

	N[ 0 ] = 1.0f;
	for ( j = 1; j <= k - 1; j++ )
	{
		left[ j ] = u - U[ i + 1 - j ];
		right[ j ] = U[ i + j ] - u;
		saved = 0.0f;
		for ( r = 0; r < j; r++ )
		{
			temp = N[ r ] / ( right[ r + 1 ] + left[ j - r ] );
			N[ r ] = saved + right[ r + 1 ] * temp;
			saved = left[ j - r ] * temp;
		}
		N[ j ] = saved;
	}
}



//---------------------------------------------------------------------
/** Return the basis functions for the specified parameter value.
 */

void CqSurfaceNURBS::DersBasisFunctions( TqFloat u, TqUint i, std::vector<TqFloat>& U, TqInt k, TqInt n, std::vector<std::vector<TqFloat> >& ders )
{
	register TqInt j, r;
	register TqFloat saved, temp;
	std::vector<TqFloat> left( k ), right( k );
	std::vector<std::vector<TqFloat> > ndu( k ), a( 2 );
	for ( j = 0; j < k; j++ )
		ndu[ j ].resize( k );
	ders.resize( n + 1 );
	for ( j = 0; j < n + 1; j++ )
		ders[ j ].resize( k );
	a[ 0 ].resize( k );
	a[ 1 ].resize( k );

	TqInt p = k - 1;

	ndu[ 0 ][ 0 ] = 1.0f;
	for ( j = 1; j <= p; j++ )
	{
		left[ j ] = u - U[ i + 1 - j ];
		right[ j ] = U[ i + j ] - u;
		saved = 0.0f;
		for ( r = 0; r < j; r++ )
		{
			ndu[ j ][ r ] = right[ r + 1 ] + left[ j - r ];
			temp = ndu[ r ][ j - 1 ] / ndu[ j ][ r ];

			ndu[ r ][ j ] = saved + right[ r + 1 ] * temp;
			saved = left[ j - r ] * temp;
		}
		ndu[ j ][ j ] = saved;
	}

	// Load the basis functions
	for ( j = 0; j <= p; j++ )
		ders[ 0 ][ j ] = ndu[ j ][ p ];

	// Compute the derivatives.
	for ( r = 0; r <= p; r ++ )
	{
		// Alternate rows in array a.
		TqInt s1 = 0;
		TqInt s2 = 1;
		a[ 0 ][ 0 ] = 1.0f;
		TqInt j1, j2;

		// Loop to compute the kth derivative
		for ( k = 1; k <= n; k++ )
		{
			TqFloat d = 0.0f;
			TqInt rk = r - k;
			TqInt pk = p - k;
			if ( r >= k )
			{
				a[ s2 ][ 0 ] = a[ s1 ][ 0 ] / ndu[ pk + 1 ][ rk ];
				d = a[ s2 ][ 0 ] * ndu[ rk ][ pk ];
			}
			if ( rk >= -1 )
				j1 = 1;
			else
				j1 = -rk;

			if ( r - 1 <= pk )
				j2 = k - 1;
			else
				j2 = p - r;

			for ( j = j1; j <= j2; j++ )
			{
				a[ s2 ][ j ] = ( a[ s1 ][ j ] - a[ s1 ][ j - 1 ] ) / ndu[ pk + 1 ][ rk + j ];
				d += a[ s2 ][ j ] * ndu[ rk + j ][ pk ];
			}
			if ( r <= pk )
			{
				a[ s2 ][ k ] = -a[ s1 ][ k - 1 ] / ndu[ pk + 1 ][ r ];
				d += a[ s2 ][ k ] * ndu[ r ][ pk ];
			}
			ders[ k ][ r ] = d;

			// Switch rows.
			j = s1;
			s1 = s2;
			s2 = j;
		}
	}
	// Multiply through the correct factors.
	r = p;
	for ( k = 1; k <= n; k++ )
	{
		for ( j = 0; j <= p; j++ )
			ders[ k ][ j ] *= r;
		r *= ( p - k );
	}
}


//---------------------------------------------------------------------
/** Evaluate the nurbs surface at parameter values u,v.
 *
 * \todo Code Review: Unused function [except by deprecated CqSurfaceNURBS::GenerateGeometricNormals() ]
 */

CqVector4D	CqSurfaceNURBS::EvaluateWithNormal( TqFloat u, TqFloat v, CqVector4D& P )
{
	CqVector4D N;
	TqInt d = 1;	// Default to 1st order derivatives.
	TqInt k, l, s, r;
	TqInt p = uDegree();
	TqInt q = vDegree();

	std::vector<std::vector<CqVector4D> > SKL( d + 1 );
	for ( k = 0; k <= d; k++ )
		SKL[ k ].resize( d + 1 );
	std::vector<std::vector<TqFloat> > Nu, Nv;
	std::vector<CqVector4D> temp( q + 1 );

	TqInt du = min( d, p );
	for ( k = p + 1; k <= d; k++ )
		for ( l = 0; l <= d - k; l++ )
			SKL[ k ][ l ] = CqVector4D( 0.0f, 0.0f, 0.0f, 1.0f );

	TqInt dv = min( d, q );
	for ( l = q + 1; l <= d; l++ )
		for ( k = 0; k <= d - l; k++ )
			SKL[ k ][ l ] = CqVector4D( 0.0f, 0.0f, 0.0f, 1.0f );

	TqUint uspan = FindSpanU( u );
	DersBasisFunctions( u, uspan, m_auKnots, m_uOrder, du, Nu );
	TqUint vspan = FindSpanV( v );
	DersBasisFunctions( v, vspan, m_avKnots, m_vOrder, dv, Nv );

	for ( k = 0; k <= du; k++ )
	{
		for ( s = 0; s <= q; s++ )
		{
			temp[ s ] = CqVector4D( 0.0f, 0.0f, 0.0f, 1.0f );
			for ( r = 0; r <= p; r++ )
				temp[ s ] = temp[ s ] + Nu[ k ][ r ] * CP( uspan - p + r, vspan - q + s );
		}
		TqInt dd = min( d - k, dv );
		for ( l = 0; l <= dd; l++ )
		{
			SKL[ k ][ l ] = CqVector4D( 0.0f, 0.0f, 0.0f, 1.0f );
			for ( s = 0; s <= q; s++ )
				SKL[ k ][ l ] = SKL[ k ][ l ] + Nv[ l ][ s ] * temp[ s ];
		}
	}
	N = SKL[ 1 ][ 0 ] % SKL[ 0 ][ 1 ];
	N.Unit();

	P = SKL[ 0 ][ 0 ];

	return ( N );
}


//---------------------------------------------------------------------
/** Insert the specified knot into the U knot vector, and refine the control points accordingly.
 * \return The number of new knots created.
 */

TqUint CqSurfaceNURBS::InsertKnotU( TqFloat u, TqInt r )
{
	TqInt n = m_cuVerts;
	TqInt k = m_auKnots.size() - 1, s = 0;
	TqInt i, j;
	TqInt p = uDegree();


	// If the specified u value falls outside the current range, then fail.
	if ( u < m_auKnots[ uDegree() ] || u > m_auKnots[ m_cuVerts ] )
		return ( 0 );

	// Calculate k as the index of the last knot value <= the specified value.
	for ( i = 0; i < static_cast<TqInt>( m_auKnots.size() ); i++ )
	{
		if ( m_auKnots[ i ] > u )
		{
			k = i - 1;
			break;
		}
	}


	// Calculate the number of knots at the insertion point with the same value as the specified knot.
	if ( u <= m_auKnots[ k ] )
	{
		s = 1;
		for ( i = k; i > static_cast<TqInt>( uDegree() ); i-- )
		{
			if ( m_auKnots[ i ] <= m_auKnots[ i - 1 ] )
				s++;
			else
				break;
		}
	}
	else
		s = 0;

	// Adjust the number of insertions to take into account the number of knots with that value already in the vector.
	if ( ( r + s ) > p + 1 )
		r = p + 1 - s;

	// If this means we don't have to do anything then exit.
	if ( r <= 0 )
		return ( 0 );

	std::vector<TqFloat> auHold(m_auKnots);
	// Reserve more space for new knots.
	m_cuVerts += r;
	m_auKnots.reserve( m_cuVerts + m_uOrder );
	// Insert r new knots.
	std::vector<TqFloat> newKnots(r, u);
	m_auKnots.insert(m_auKnots.begin()+(k+1), newKnots.begin(), newKnots.end());

	// Now process all the 'vertex' class variables.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
	{
		if ( ( *iUP ) ->Class() == class_vertex )
		{
			CqParameter * pHold = ( *iUP ) ->Clone();
			( *iUP ) ->SetSize( m_cuVerts * m_cvVerts );

			// Save unaltered control points
			CqParameter* R = ( *iUP ) ->CloneType( "R" );
			R->SetSize( p + 1 );

			// Insert control points as required on each row.
			TqUint row;
			for ( row = 0; row < m_cvVerts; row++ )
			{
				// First copy the first set of control points up to the insertion point minus the degree
				for ( i = 0; i <= k - p; i++ )
					// Qw[i][row] = Pw[i][row]
					( *iUP ) ->SetValue( pHold, ( row * m_cuVerts ) + i, ( row * n ) + i );
				for ( i = k - s; i < static_cast<TqInt>( m_cuVerts ) - r; i++ )
					// Qw[i+r][row] = Pw[i][row]
					( *iUP ) ->SetValue( pHold, ( row * m_cuVerts ) + i + r, ( row * n ) + i );
				for ( i = 0; i <= p - s; i++ )
					// Rw[i] = Pw[k-p+i][row]
					R->SetValue( pHold, i, ( row * n ) + k - p + i );

				// Insert the knot r times
				TqUint L = 0 ;
				TqFloat alpha;
				for ( j = 1; j <= r; j++ )
				{
					L = k - p + j;
					for ( i = 0;i <= p - j - s;i++ )
					{
						alpha = ( u - auHold[ L + i ] ) / ( auHold[ i + k + 1 ] - auHold[ L + i ] );

						switch ( ( *iUP ) ->Type() )
						{
								case type_float:
								{
									CqParameterTyped<TqFloat, TqFloat>* pTR = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( R );
									( *pTR->pValue( i ) ) = alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) );
									break;
								}

								case type_integer:
								{
									CqParameterTyped<TqInt, TqFloat>* pTR = static_cast<CqParameterTyped<TqInt, TqFloat>*>( R );
									( *pTR->pValue( i ) ) = static_cast<TqInt> (
									                            alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) )
									                        );
									break;
								}

								case type_point:
								case type_normal:
								case type_vector:
								{
									CqParameterTyped<CqVector3D, CqVector3D>* pTR = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( R );
									( *pTR->pValue( i ) ) = alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) );
									break;
								}

								case type_hpoint:
								{
									CqParameterTyped<CqVector4D, CqVector3D>* pTR = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( R );
									CqVector4D cp( alpha * ( *pTR->pValue( i + 1 ) ).x() + ( 1.0f - alpha ) * ( *pTR->pValue( i ) ).x(),
									               alpha * ( *pTR->pValue( i + 1 ) ).y() + ( 1.0f - alpha ) * ( *pTR->pValue( i ) ).y(),
									               alpha * ( *pTR->pValue( i + 1 ) ).z() + ( 1.0f - alpha ) * ( *pTR->pValue( i ) ).z(),
									               alpha * ( *pTR->pValue( i + 1 ) ).h() + ( 1.0f - alpha ) * ( *pTR->pValue( i ) ).h() );
									( *pTR->pValue( i ) ) = cp;
									break;
								}

								case type_color:
								{
									CqParameterTyped<CqColor, CqColor>* pTR = static_cast<CqParameterTyped<CqColor, CqColor>*>( R );
									( *pTR->pValue( i ) ) = alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) );
									break;
								}

								//							case type_string:
								//							{
								//								CqParameterTyped<CqString, CqString>* pTR = static_cast<CqParameterTyped<CqString, CqString>*>(R);
								//								(*pTR->pValue( i )) = alpha * (*pTR->pValue( i + 1 )) + ( 1.0 - alpha ) * (*pTR->pValue( i ));
								//								break;
								//							}

								case type_matrix:
								{
									CqParameterTyped<CqMatrix, CqMatrix>* pTR = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( R );
									( *pTR->pValue( i ) ) = alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) );
									break;
								}

								default:
								{
									// left blank to avoid compiler warnings about unhandled types in the switch
									break;
								}
						}
					}
					// Qw[L][row] = Rw[0]
					( *iUP ) ->SetValue( R, ( row * m_cuVerts ) + L, 0 );
					if ( p - j - s > 0 )
						// Qw[k+r-j-s][row] = Rw[p-j-s]
						( *iUP ) ->SetValue( R, ( row * m_cuVerts ) + k + r - j - s, p - j - s );
				}

				// Load remaining control points
				for ( i = L + 1; i < k - s; i++ )
					// Qw[i][row] = Rw[i-L]
					( *iUP ) ->SetValue( R, ( row * m_cuVerts ) + i, i - L );
			}

			delete( R );
			delete( pHold );
		}
	}
	return ( r );
}


//---------------------------------------------------------------------
/** Insert the specified knot into the V knot vector, and refine the control points accordingly.
 * \return The number of new knots created.
 */

TqUint CqSurfaceNURBS::InsertKnotV( TqFloat v, TqInt r )
{
	// Compute k and s      v = [ v_k , v_k+1)  with v_k having multiplicity s
	TqUint m = static_cast<TqUint>( m_cvVerts );
	TqInt k = m_avKnots.size() - 1, s = 0;
	TqInt i, j;
	TqInt p = vDegree();

	if ( v < m_avKnots[ vDegree() ] || v > m_avKnots[ m_cvVerts ] )
		return ( 0 );

	TqInt size = static_cast<TqInt>( m_avKnots.size() );
	for ( i = 0; i < size; i++ )
	{
		if ( m_avKnots[ i ] > v )
		{
			k = i - 1;
			break;
		}
	}

	if ( v <= m_avKnots[ k ] )
	{
		s = 1;
		for ( i = k; i > static_cast<TqInt>( vDegree() ); i-- )
		{
			if ( m_avKnots[ i ] <= m_avKnots[ i - 1 ] )
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

	std::vector<TqFloat> avHold(m_avKnots);
	// Reserve more space for new knots.
	m_cvVerts += r;
	m_avKnots.reserve( m_cvVerts + m_vOrder );
	// Insert r new knots
	std::vector<TqFloat> newKnots(r, v);
	m_avKnots.insert(m_avKnots.begin()+(k+1), newKnots.begin(), newKnots.end());

	// Now process all the 'vertex' class variables.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
	{
		if ( ( *iUP ) ->Class() == class_vertex )
		{
			CqParameter * pHold = ( *iUP ) ->Clone();
			( *iUP ) ->SetSize( m_cuVerts * m_cvVerts );

			// Save unaltered control points
			CqParameter* R = ( *iUP ) ->CloneType( "R" );
			R->SetSize( p + 1 );

			// Insert control points as required on each row.
			TqUint col;
			for ( col = 0; col < m_cuVerts; col++ )
			{
				for ( i = 0; i <= k - p; i++ )
					// Qw[col][i] = Pw[col][i]
					( *iUP ) ->SetValue( pHold, ( i * m_cuVerts ) + col, ( i * m_cuVerts ) + col );
				for ( i = k - s; i < static_cast<TqInt>( m ); i++ )
					// Qw[col][i+r] = Pw[col][i]
					( *iUP ) ->SetValue( pHold, ( ( i + r ) * m_cuVerts ) + col, ( i * m_cuVerts ) + col );
				for ( i = 0; i <= p - s; i++ )
					// Rw[i] = Pw[col][k-p+i]
					R->SetValue( pHold, i, ( ( k - p + i ) * m_cuVerts ) + col );

				// Insert the knot r times
				TqUint L = 0 ;
				TqFloat alpha;
				for ( j = 1; j <= r; j++ )
				{
					L = k - p + j;
					for ( i = 0;i <= p - j - s;i++ )
					{
						alpha = ( v - avHold[ L + i ] ) / ( avHold[ i + k + 1 ] - avHold[ L + i ] );

						switch ( ( *iUP ) ->Type() )
						{
								case type_float:
								{
									CqParameterTyped<TqFloat, TqFloat>* pTR = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( R );
									( *pTR->pValue( i ) ) = alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) );
									break;
								}

								case type_integer:
								{
									CqParameterTyped<TqInt, TqFloat>* pTR = static_cast<CqParameterTyped<TqInt, TqFloat>*>( R );
									( *pTR->pValue( i ) ) = static_cast<TqInt> (
									                            alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) )
									                        );
									break;
								}

								case type_point:
								case type_normal:
								case type_vector:
								{
									CqParameterTyped<CqVector3D, CqVector3D>* pTR = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( R );
									( *pTR->pValue( i ) ) = alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) );
									break;
								}

								case type_hpoint:
								{
									CqParameterTyped<CqVector4D, CqVector3D>* pTR = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( R );
									CqVector4D cp( alpha * ( *pTR->pValue( i + 1 ) ).x() + ( 1.0f - alpha ) * ( *pTR->pValue( i ) ).x(),
									               alpha * ( *pTR->pValue( i + 1 ) ).y() + ( 1.0f - alpha ) * ( *pTR->pValue( i ) ).y(),
									               alpha * ( *pTR->pValue( i + 1 ) ).z() + ( 1.0f - alpha ) * ( *pTR->pValue( i ) ).z(),
									               alpha * ( *pTR->pValue( i + 1 ) ).h() + ( 1.0f - alpha ) * ( *pTR->pValue( i ) ).h() );
									( *pTR->pValue( i ) ) = cp;
									break;
								}

								case type_color:
								{
									CqParameterTyped<CqColor, CqColor>* pTR = static_cast<CqParameterTyped<CqColor, CqColor>*>( R );
									( *pTR->pValue( i ) ) = alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) );
									break;
								}

								//							case type_string:
								//							{
								//								CqParameterTyped<CqString, CqString>* pTR = static_cast<CqParameterTyped<CqString, CqString>*>(R);
								//								(*pTR->pValue( i )) = alpha * (*pTR->pValue( i + 1 )) + ( 1.0 - alpha ) * (*pTR->pValue( i ));
								//								break;
								//							}

								case type_matrix:
								{
									CqParameterTyped<CqMatrix, CqMatrix>* pTR = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( R );
									( *pTR->pValue( i ) ) = alpha * ( *pTR->pValue( i + 1 ) ) + ( 1.0 - alpha ) * ( *pTR->pValue( i ) );
									break;
								}

								default:
								{
									// left blank to avoid compiler warnings about unhandled types
									break;
								}
						}
					}
					// Qw[col][L] = Rw[0]
					( *iUP ) ->SetValue( R, ( L * m_cuVerts ) + col, 0 );
					if ( p - j - s > 0 )
						// Qw[col][k+r-j-s] = Rw[p-j-s]
						( *iUP ) ->SetValue( R, ( ( k + r - j - s ) * m_cuVerts ) + col, p - j - s );
				}

				// Load remaining control points
				for ( i = L + 1; i < k - s; i++ )
					// Qw[col][i] = Rw[i-L]
					( *iUP ) ->SetValue( R, ( i * m_cuVerts ) + col, i - L );
			}

			delete( R );
			delete( pHold );
		}
	}
	return ( r );
}



//---------------------------------------------------------------------
/** Insert the specified knots into the U knot vector, and refine the control points accordingly.
 */

void CqSurfaceNURBS::RefineKnotU( const std::vector<TqFloat>& X )
{
	if ( X.size() <= 0 )
		return ;

	TqInt n = m_cuVerts - 1;
	TqInt p = uDegree();
	TqInt m = n + p + 1;
	TqInt a, b;
	TqInt r = X.size() - 1;
	TqInt j, row;

	// Find the insertion points for the start and end of the knot vector to be inserted.
	a = FindSpanU( X[ 0 ] );
	b = FindSpanU( X[ r ] );
	++b;

	TqInt i = b + p - 1;
	TqInt k = b + p + r;

	m_cuVerts = r + 1 + n + 1;
	std::vector<TqFloat>	auHold( m_auKnots );
	m_auKnots.resize( m_cuVerts + m_uOrder );

	// Copy the knot values up to the first insertion point.
	for ( j = 0; j <= a; j++ )
		m_auKnots[ j ] = auHold[ j ];
	// Copy the knot values from the second insertion point to the end.
	for ( j = b + p; j <= m; j++ )
		m_auKnots[ j + r + 1 ] = auHold[ j ];
	for ( j = r; j >= 0; j-- )
	{
		while ( X[ j ] <= m_auKnots[ i ] && i > a )
			m_auKnots[ k-- ] = auHold[ i-- ];
		m_auKnots[ k-- ] = X[ j ];
	}


	i = b + p - 1;
	k = b + p + r;



	// Now process all the 'vertex' class variables.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end ; iUP++ )
	{
		if ( ( *iUP ) ->Class() == class_vertex )
		{
			i = b + p - 1;
			k = b + p + r;

			CqParameter* pHold = ( *iUP ) ->Clone();
			( *iUP ) ->SetSize( m_cuVerts * m_cvVerts );

			// Copy the control points from the original
			for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
			{
				// Copy the CPs up to the first insertion point minus the degree (this is the number of control points in that section).
				TqUint rowoff = ( row * m_cuVerts );
				for ( j = 0; j <= a - p ; j++ )
					( *iUP ) ->SetValue( pHold, rowoff + j, ( row * ( n + 1 ) ) + j );
				// Copy the CPs beyond the second insertion point to the end.
				for ( j = b - 1; j <= n; j++ )
					( *iUP ) ->SetValue( pHold, rowoff + j + r + 1, ( row * ( n + 1 ) ) + j );
			}

			for ( j = r; j >= 0; j-- )
			{
				while ( X[ j ] <= m_auKnots[ i ] && i > a )
				{
					for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
						( *iUP ) ->SetValue( pHold, ( row * m_cuVerts ) + k - p - 1, ( row * ( n + 1 ) ) + i - p - 1 );
					--k;
					--i;
				}

				for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
					( *iUP ) ->SetValue( ( *iUP ), ( row * m_cuVerts ) + k - p - 1, ( row * m_cuVerts ) + k - p );

				TqInt l;
				for ( l = 1; l <= p ; l++ )
				{
					TqUint ind = k - p + l;
					TqFloat alpha = m_auKnots[ k + l ] - X[ j ];
					if ( alpha == 0.0 )
					{
						for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
							( *iUP ) ->SetValue( ( *iUP ), ( row * m_cuVerts ) + ind - 1 , ( row * m_cuVerts ) + ind );
					}
					else
					{
						alpha /= m_auKnots[ k + l ] - auHold[ i - p + l ];
						// Make sure index is OK.
						switch ( ( *iUP ) ->Type() )
						{
								case type_float:
								{
									CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );
									for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
										pTParam->pValue( ( row * m_cuVerts ) + ind - 1 ) [ 0 ] = alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ];
									break;
								}

								case type_integer:
								{
									CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( ( *iUP ) );
									for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
										pTParam->pValue( ( row * m_cuVerts ) + ind - 1 ) [ 0 ] =
										    static_cast<TqInt>(
										        alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ]
										    );
									break;
								}

								case type_point:
								case type_normal:
								case type_vector:
								{
									CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
									for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
										pTParam->pValue( ( row * m_cuVerts ) + ind - 1 ) [ 0 ] = alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ];
									break;
								}

								case type_hpoint:
								{
									CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
									for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
									{
										CqVector4D cp( alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ].x() + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ].x(),
										               alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ].y() + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ].y(),
										               alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ].z() + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ].z(),
										               alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ].h() + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ].h() );
										pTParam->pValue( ( row * m_cuVerts ) + ind - 1 ) [ 0 ] = cp;
									}
									break;
								}

								case type_color:
								{
									CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );
									for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
										pTParam->pValue( ( row * m_cuVerts ) + ind - 1 ) [ 0 ] = alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ];
									break;
								}

								case type_string:
								{
									CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( ( *iUP ) );
									for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
										pTParam->pValue( ( row * m_cuVerts ) + ind - 1 ) [ 0 ] = alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ];
									break;
								}

								case type_matrix:
								{
									CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( ( *iUP ) );
									for ( row = 0; row < static_cast<TqInt>( m_cvVerts ); row++ )
										pTParam->pValue( ( row * m_cuVerts ) + ind - 1 ) [ 0 ] = alpha * pTParam->pValue() [ ( row * m_cuVerts ) + ind - 1 ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( row * m_cuVerts ) + ind ];
									break;
								}

								default:
								{
									// left blank to avoid compiler warnings about unhandled types
									break;
								}
						}
					}
				}
				--k;
			}
			delete( pHold );
		}
	}
}


//---------------------------------------------------------------------
/** Insert the specified knots into the V knot vector, and refine the control points accordingly.
 */

void CqSurfaceNURBS::RefineKnotV( const std::vector<TqFloat>& X )
{
	if ( X.size() <= 0 )
		return ;

	TqInt n = m_cvVerts - 1;
	TqInt p = vDegree();
	TqInt m = n + p + 1;
	TqInt a, b;
	TqInt r = X.size() - 1;
	TqInt j, col;

	a = FindSpanV( X[ 0 ] ) ;
	b = FindSpanV( X[ r ] ) ;
	++b;

	TqInt i = b + p - 1;
	TqInt k = b + p + r;

	m_cvVerts = r + 1 + n + 1;
	std::vector<TqFloat>	avHold( m_avKnots );
	m_avKnots.resize( m_cvVerts + m_vOrder );

	for ( j = 0; j <= a; j++ )
		m_avKnots[ j ] = avHold[ j ];
	for ( j = b + p; j <= m; j++ )
		m_avKnots[ j + r + 1 ] = avHold[ j ];
	for ( j = r; j >= 0 ; j-- )
	{
		while ( X[ j ] <= m_avKnots[ i ] && i > a )
			m_avKnots[ k-- ] = avHold[ i-- ];
		m_avKnots[ k-- ] = X[ j ];
	}


	i = b + p - 1;
	k = b + p + r;


	// Now process all the 'vertex' class variables.
	std::vector<CqParameter*>::iterator iUP;
	std::vector<CqParameter*>::iterator end = m_aUserParams.end();
	for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
	{
		if ( ( *iUP ) ->Class() == class_vertex )
		{
			i = b + p - 1;
			k = b + p + r;

			CqParameter* pHold = ( *iUP ) ->Clone();
			( *iUP ) ->SetSize( m_cuVerts * m_cvVerts );

			for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
			{
				for ( j = 0; j <= a - p; j++ )
					( *iUP ) ->SetValue( pHold, ( j * m_cuVerts ) + col, ( j * m_cuVerts ) + col );
				for ( j = b - 1; j <= n; j++ )
					( *iUP ) ->SetValue( pHold, ( ( j + r + 1 ) * m_cuVerts ) + col, ( j * m_cuVerts ) + col );
			}


			for ( j = r; j >= 0 ; j-- )
			{
				while ( X[ j ] <= m_avKnots[ i ] && i > a )
				{
					for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
						( *iUP ) ->SetValue( pHold, ( ( k - p - 1 ) * m_cuVerts ) + col, ( ( i - p - 1 ) * m_cuVerts ) + col );
					--k;
					--i;
				}
				for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
					( *iUP ) ->SetValue( ( *iUP ), ( ( k - p - 1 ) * m_cuVerts ) + col, ( ( k - p ) * m_cuVerts ) + col );

				TqInt l;
				for ( l = 1; l <= p; l++ )
				{
					TqUint ind = k - p + l;
					TqFloat alpha = m_avKnots[ k + l ] - X[ j ];
					if ( alpha == 0.0 )
					{
						for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
							( *iUP ) ->SetValue( ( *iUP ), ( ( ind - 1 ) * m_cuVerts ) + col, ( ind * m_cuVerts ) + col );
					}
					else
					{
						alpha /= m_avKnots[ k + l ] - avHold[ i - p + l ];
						switch ( ( *iUP ) ->Type() )
						{
								case type_float:
								{
									CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( ( *iUP ) );
									for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
										pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] = alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ];
									break;
								}

								case type_integer:
								{
									CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( ( *iUP ) );
									for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
										pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] = static_cast<TqInt>(
										            alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ]
										        );
									break;
								}

								case type_point:
								case type_normal:
								case type_vector:
								{
									CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( ( *iUP ) );
									for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
										pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] = alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ];
									break;
								}

								case type_hpoint:
								{
									CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( ( *iUP ) );
									for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
									{
										CqVector4D cp( alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ].x() + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ].x(),
										               alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ].y() + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ].y(),
										               alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ].z() + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ].z(),
										               alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ].h() + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ].h() );
										pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] = cp;
									}
									break;
								}

								case type_color:
								{
									CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( ( *iUP ) );
									for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
										pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] = alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ];
									break;
								}

								case type_string:
								{
									CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( ( *iUP ) );
									for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
										pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] = alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ];
									break;
								}

								case type_matrix:
								{
									CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( ( *iUP ) );
									for ( col = 0; col < static_cast<TqInt>( m_cuVerts ); col++ )
										pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] = alpha * pTParam->pValue() [ ( ( ind - 1 ) * m_cuVerts ) + col ] + ( 1.0f - alpha ) * pTParam->pValue() [ ( ind * m_cuVerts ) + col ];
									break;
								}

								default:
								{
									// left blank to avoid compiler warnings about unhandled types
									break;
								}
						}
					}
				}
				--k;
			}
			delete( pHold );
		}
	}
}


//---------------------------------------------------------------------
/** Ensure a nonperiodic (clamped) knot vector by inserting U[p] and U[m-p] multiple times.
 */

void CqSurfaceNURBS::ClampU()
{
	TqFloat u1 = m_auKnots[ uDegree() ];
	TqFloat u2 = m_auKnots[ m_cuVerts ];

	TqUint n1 = InsertKnotU( u1, uDegree() );
	TqUint n2 = InsertKnotU( u2, uDegree() );

	// Now trim unnecessary knots and control points
	if ( n1 || n2 )
	{
		std::vector<TqFloat> auHold( m_auKnots );
		m_auKnots.resize( m_auKnots.size() - n1 - n2 );

		TqUint i;
		for ( i = n1; i < auHold.size() - n2; i++ )
			m_auKnots[ i - n1 ] = auHold[ i ];

		TqInt n = m_cuVerts;
		m_cuVerts -= n1 + n2;

		// Now process all the 'vertex' class variables.
		std::vector<CqParameter*>::iterator iUP;
		std::vector<CqParameter*>::iterator end = m_aUserParams.end();
		for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
		{
			if ( ( *iUP ) ->Class() == class_vertex )
			{
				CqParameter * pHold = ( *iUP ) ->Clone();
				( *iUP ) ->SetSize( ( m_cuVerts ) * m_cvVerts );

				TqUint row;
				for ( row = 0; row < m_cvVerts; row++ )
				{
					TqUint i;
					for ( i = n1; i < n - n2; i++ )
						( *iUP ) ->SetValue( pHold, ( row * m_cuVerts ) + i - n1, ( row * n ) + i );
				}
				delete( pHold );
			}
		}
	}
}


//---------------------------------------------------------------------
/** Ensure a nonperiodic (clamped) knot vector by inserting V[p] and V[m-p] multiple times.
 */

void CqSurfaceNURBS::ClampV()
{
	TqFloat v1 = m_avKnots[ vDegree() ];
	TqFloat v2 = m_avKnots[ m_cvVerts ];

	TqUint n1 = InsertKnotV( v1, vDegree() );
	TqUint n2 = InsertKnotV( v2, vDegree() );

	// Now trim unnecessary knots and control points
	if ( n1 || n2 )
	{
		std::vector<TqFloat> avHold( m_avKnots );
		m_avKnots.resize( m_avKnots.size() - n1 - n2 );

		TqUint i;
		for ( i = n1; i < avHold.size() - n2; i++ )
			m_avKnots[ i - n1 ] = avHold[ i ];

		TqInt n = m_cvVerts;
		m_cvVerts -= n1 + n2;

		// Now process all the 'vertex' class variables.
		std::vector<CqParameter*>::iterator iUP;
		std::vector<CqParameter*>::iterator end = m_aUserParams.end();
		for ( iUP = m_aUserParams.begin(); iUP != end; iUP++ )
		{
			if ( ( *iUP ) ->Class() == class_vertex )
			{
				CqParameter * pHold = ( *iUP ) ->Clone();
				( *iUP ) ->SetSize( ( m_cvVerts ) * m_cuVerts );

				TqUint col;
				for ( col = 0; col < m_cuVerts; col++ )
				{
					TqUint i;
					for ( i = n1; i < n - n2; i++ )
						( *iUP ) ->SetValue( pHold, ( ( i - n1 ) * m_cuVerts ) + col, ( i * m_cuVerts ) + col );
				}
				delete( pHold );
			}
		}
	}
}


//---------------------------------------------------------------------
/** Split this NURBS surface into two subsurfaces along u or v depending on dirflag (TRUE=u)
 */

void CqSurfaceNURBS::SplitNURBS( CqSurfaceNURBS& nrbA, CqSurfaceNURBS& nrbB, bool dirflag )
{
	std::vector<TqFloat>& aKnots = ( dirflag ) ? m_auKnots : m_avKnots;
	TqUint Order = ( dirflag ) ? m_uOrder : m_vOrder;

	// Add a multiplicty k knot to the knot vector in the direction
	// specified by dirflag, and refine the surface.  This creates two
	// adjacent surfaces with c0 discontinuity at the seam.
	TqUint extra = 0L;
	TqUint last = ( dirflag ) ? ( m_cuVerts + m_uOrder - 1 ) : ( m_cvVerts + m_vOrder - 1 );
	//    TqUint middex=last/2;
	//    TqFloat midVal=aKnots[middex];
	TqFloat midVal = ( aKnots[ 0 ] + aKnots[ last ] ) / 2;
	TqUint middex = ( dirflag ) ? FindSpanU( midVal ) : FindSpanV( midVal );

	// Search forward and backward to see if multiple knot is already there
	TqUint i = 0;
	TqUint same = 0L;
	if ( aKnots[ middex ] == midVal )
	{
		i = middex + 1L;
		same = 1L;
		while ( ( i < last ) && ( aKnots[ i ] == midVal ) )
		{
			i++;
			same++;
		}

		i = middex - 1L;
		while ( ( i > 0L ) && ( aKnots[ i ] == midVal ) )
		{
			i--;
			middex--;	// middex is start of multiple knot
			same++;
		}
	}

	if ( i <= 0L )           	    // No knot in middle, must create it
	{
		midVal = ( aKnots[ 0L ] + aKnots[ last ] ) / 2.0;
		middex = 0;
		while ( aKnots[ middex + 1L ] < midVal )
			middex++;
		same = 0L;
	}

	extra = Order - same;
	std::vector<TqFloat> anewKnots( extra );

	if ( same < Order )           	    // Must add knots
	{
		for ( i = 0; i < extra; i++ )
			anewKnots[ i ] = midVal;
	}

	TqUint SplitPoint = ( extra < Order ) ? middex - 1L : middex;
	if ( dirflag )
		RefineKnotU( anewKnots );
	else
		RefineKnotV( anewKnots );

	// Build the two child surfaces, and copy the data from the refined
	// version of the parent (tmp) into the two children

	// First half
	nrbA.Init( m_uOrder, m_vOrder, ( dirflag ) ? SplitPoint + 1L : m_cuVerts, ( dirflag ) ? m_cvVerts : SplitPoint + 1L );
	TqUint j;
	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if ( ( *iUP ) ->Class() == class_vertex )
		{
			CqParameter * pNewA = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
			pNewA->SetSize( nrbA.cuVerts() * nrbA.cvVerts() );
			for ( i = 0L; i < nrbA.m_cvVerts; i++ )
				for ( j = 0L; j < nrbA.m_cuVerts; j++ )
					pNewA->SetValue( ( *iUP ), ( i * nrbA.cuVerts() ) + j, ( i * m_cuVerts ) + j );
			nrbA.AddPrimitiveVariable( pNewA );
		}
	}

	for ( i = 0L; i < nrbA.m_uOrder + nrbA.m_cuVerts; i++ )
		nrbA.m_auKnots[ i ] = m_auKnots[ i ];
	for ( i = 0L; i < nrbA.m_vOrder + nrbA.m_cvVerts; i++ )
		nrbA.m_avKnots[ i ] = m_avKnots[ i ];

	// Second half
	SplitPoint++;
	nrbB.Init( m_uOrder, m_vOrder, ( dirflag ) ? m_cuVerts - SplitPoint : m_cuVerts, ( dirflag ) ? m_cvVerts : m_cvVerts - SplitPoint );
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if ( ( *iUP ) ->Class() == class_vertex )
		{
			CqParameter * pNewB = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
			pNewB->SetSize( nrbB.cuVerts() * nrbB.cvVerts() );
			for ( i = 0L; i < nrbB.m_cvVerts; i++ )
			{
				for ( j = 0L; j < nrbB.m_cuVerts; j++ )
				{
					TqUint iSrc = ( dirflag ) ? i : i + SplitPoint;
					iSrc *= m_cuVerts;
					iSrc += ( dirflag ) ? j + SplitPoint : j;
					pNewB->SetValue( ( *iUP ), ( i * nrbB.cuVerts() + j ), iSrc );
				}
			}
			nrbB.AddPrimitiveVariable( pNewB );
		}
	}

	for ( i = 0L; i < nrbB.m_uOrder + nrbB.m_cuVerts; i++ )
		nrbB.m_auKnots[ i ] = m_auKnots[ ( dirflag ) ? ( i + SplitPoint ) : i ];
	for ( i = 0L; i < nrbB.m_vOrder + nrbB.m_cvVerts; i++ )
		nrbB.m_avKnots[ i ] = m_avKnots[ ( dirflag ) ? i : ( i + SplitPoint ) ];
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the u direction, return the left side.
 */

void CqSurfaceNURBS::uSubdivide( CqSurfaceNURBS*& pnrbA, CqSurfaceNURBS*& pnrbB )
{
	pnrbA = new CqSurfaceNURBS();
	pnrbB = new CqSurfaceNURBS();

	SplitNURBS( *pnrbA, *pnrbB, true );

	uSubdivideUserParameters( pnrbA, pnrbB );
}


//---------------------------------------------------------------------
/** Subdivide a bicubic patch in the v direction, return the top side.
 */

void CqSurfaceNURBS::vSubdivide( CqSurfaceNURBS*& pnrbA, CqSurfaceNURBS*& pnrbB )
{
	pnrbA = new CqSurfaceNURBS();
	pnrbB = new CqSurfaceNURBS();

	SplitNURBS( *pnrbA, *pnrbB, false );

	vSubdivideUserParameters( pnrbA, pnrbB );
}


//---------------------------------------------------------------------
/** Return the boundary extents in object space of the surface patch
 */

void CqSurfaceNURBS::Bound(CqBound* bound) const
{
	// Get the boundary in camera space.
	CqVector3D	vecA( FLT_MAX, FLT_MAX, FLT_MAX );
	CqVector3D	vecB( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	TqUint i;
	for ( i = 0; i < m_cuVerts*m_cvVerts; i++ )
	{
		CqVector3D	vecV = vectorCast<CqVector3D>(P()->pValue( i )[0]);
		if ( vecV.x() < vecA.x() )
			vecA.x( vecV.x() );
		if ( vecV.y() < vecA.y() )
			vecA.y( vecV.y() );
		if ( vecV.x() > vecB.x() )
			vecB.x( vecV.x() );
		if ( vecV.y() > vecB.y() )
			vecB.y( vecV.y() );
		if ( vecV.z() < vecA.z() )
			vecA.z( vecV.z() );
		if ( vecV.z() > vecB.z() )
			vecB.z( vecV.z() );
	}
	bound->vecMin() = vecA;
	bound->vecMax() = vecB;

	AdjustBoundForTransformationMotion( bound );
}


//---------------------------------------------------------------------
/** Dice the patch into a mesh of micropolygons.
 */

void CqSurfaceNURBS::NaturalDice( CqParameter* pParameter, TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pData )
{
	assert(pParameter->Count() == pData->ArrayLength());
	CqVector4D vec1;
	TqInt iv;
	for ( iv = 0; iv <= vDiceSize; iv++ )
	{
		TqFloat sv = ( static_cast<TqFloat>( iv ) / static_cast<TqFloat>( vDiceSize ) )
		             * ( m_avKnots[ m_cvVerts ] - m_avKnots[ m_vOrder - 1 ] )
		             + m_avKnots[ m_vOrder - 1 ];
		TqInt iu;
		for ( iu = 0; iu <= uDiceSize; iu++ )
		{
			TqInt igrid = ( iv * ( uDiceSize + 1 ) ) + iu;
			TqFloat su = ( static_cast<TqFloat>( iu ) / static_cast<TqFloat>( uDiceSize ) )
			             * ( m_auKnots[ m_cuVerts ] - m_auKnots[ m_uOrder - 1 ] )
			             + m_auKnots[ m_uOrder - 1 ];

			switch ( pParameter->Type() )
			{
					case type_float:
					{
						CqParameterTyped<TqFloat, TqFloat>* pTParam = static_cast<CqParameterTyped<TqFloat, TqFloat>*>( pParameter );
						IqShaderData* arrayValue;
						TqInt i;
						for(i = 0; i<pParameter->Count(); i++)
						{
							arrayValue = pData->ArrayEntry(i);
							arrayValue->SetValue( Evaluate(su, sv, pTParam, i), igrid);
						}
						break;
					}

					case type_integer:
					{
						CqParameterTyped<TqInt, TqFloat>* pTParam = static_cast<CqParameterTyped<TqInt, TqFloat>*>( pParameter );
						IqShaderData* arrayValue;
						TqInt i;
						for(i = 0; i<pParameter->Count(); i++)
						{
							arrayValue = pData->ArrayEntry(i);
							arrayValue->SetValue( Evaluate(su, sv, pTParam, i), igrid);
						}
						break;
					}

					case type_point:
					case type_normal:
					case type_vector:
					{
						CqParameterTyped<CqVector3D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector3D, CqVector3D>*>( pParameter );
						IqShaderData* arrayValue;
						TqInt i;
						for(i = 0; i<pParameter->Count(); i++)
						{
							arrayValue = pData->ArrayEntry(i);
							arrayValue->SetValue( Evaluate(su, sv, pTParam, i), igrid);
						}
						break;
					}

					case type_hpoint:
					{
						CqParameterTyped<CqVector4D, CqVector3D>* pTParam = static_cast<CqParameterTyped<CqVector4D, CqVector3D>*>( pParameter );
						IqShaderData* arrayValue;
						TqInt i;
						for(i = 0; i<pParameter->Count(); i++)
						{
							arrayValue = pData->ArrayEntry(i);
							arrayValue->SetValue(vectorCast<CqVector3D>(Evaluate(su, sv, pTParam)), igrid);
						}
						break;
					}

					case type_color:
					{
						CqParameterTyped<CqColor, CqColor>* pTParam = static_cast<CqParameterTyped<CqColor, CqColor>*>( pParameter );
						IqShaderData* arrayValue;
						TqInt i;
						for(i = 0; i<pParameter->Count(); i++)
						{
							arrayValue = pData->ArrayEntry(i);
							arrayValue->SetValue( Evaluate(su, sv, pTParam, i), igrid);
						}
						break;
					}

					case type_string:
					{
						CqParameterTyped<CqString, CqString>* pTParam = static_cast<CqParameterTyped<CqString, CqString>*>( pParameter );
						IqShaderData* arrayValue;
						TqInt i;
						for(i = 0; i<pParameter->Count(); i++)
						{
							arrayValue = pData->ArrayEntry(i);
							arrayValue->SetValue( Evaluate(su, sv, pTParam, i), igrid);
						}
						break;
					}

					case type_matrix:
					{
						CqParameterTyped<CqMatrix, CqMatrix>* pTParam = static_cast<CqParameterTyped<CqMatrix, CqMatrix>*>( pParameter );
						IqShaderData* arrayValue;
						TqInt i;
						for(i = 0; i<pParameter->Count(); i++)
						{
							arrayValue = pData->ArrayEntry(i);
							arrayValue->SetValue( Evaluate(su, sv, pTParam, i), igrid);
						}
						break;
					}

					default:
					{
						// left blank to avoid compiler warnings about unhandled types
						break;
					}
			}
		}
	}
}


//---------------------------------------------------------------------
/** Generate the vertex normals if not specified.
 */

/*

// This function used to generate the normals for NURBS, but was disabled in
// the old cvs repository:
//
//   Revision 1.19 - (view) (download) (annotate) - [select for diffs]
//   Tue Oct 8 20:10:41 2002 UTC (5 years, 8 months ago) by pgregory
//   Branch: MAIN
//   Changes since 1.18: +2 -1 lines
//   Diff to previous 1.18
//
//   Fix some bugs in the 'clamping' of NURBS surfaces.
//   Disable generation of normals as there is a problem with it at the moment. Let the grid generate normals until it is fixed.
//
// If this code was to be resurrected, it should be moved into the more modern
// DiceAll() function, which would also resolve the todo.  We'd need to recall
// what the "problem" was with the old version - possibly something to do with
// degenerate control points and tangents?


void CqSurfaceNURBS::GenerateGeometricNormals( TqInt uDiceSize, TqInt vDiceSize, IqShaderData* pNormals )
{
	// Get the handedness of the coordinate system (at the time of creation) and
	// the coordinate system specified, to check for normal flipping.
	assert( NULL != P() );

	bool CSO = pTransform()->GetHandedness(pTransform()->Time(0));
	bool O = pAttributes() ->GetIntegerAttribute( "System", "Orientation" ) [ 0 ] != 0;

	CqVector3D	N;
	CqVector4D P;
	TqInt iv, iu;
	for ( iv = 0; iv <= vDiceSize; iv++ )
	{
		TqFloat sv = ( static_cast<TqFloat>( iv ) / static_cast<TqFloat>( vDiceSize ) )
		             * ( m_avKnots[ m_cvVerts ] - m_avKnots[ m_vOrder - 1 ] )
		             + m_avKnots[ m_vOrder - 1 ];
		for ( iu = 0; iu <= uDiceSize; iu++ )
		{
			TqFloat su = ( static_cast<TqFloat>( iu ) / static_cast<TqFloat>( uDiceSize ) )
			             * ( m_auKnots[ m_cuVerts ] - m_auKnots[ m_uOrder - 1 ] )
			             + m_auKnots[ m_uOrder - 1 ];
			TqInt igrid = ( iv * ( uDiceSize + 1 ) ) + iu;
			N = EvaluateWithNormal( su, sv, P );
			N = ( (O && CSO) || (!O && !CSO) ) ? N : -N;
			pNormals->SetNormal( N, igrid );
			/// \todo This would be more efficient if we can store the P here as well, instead of calculating it twice.
			//pP->SetPoint( P, igrid );
		}
	}
}

*/

//---------------------------------------------------------------------
/** Split the patch into smaller patches.
 */

TqInt CqSurfaceNURBS::Split( std::vector<boost::shared_ptr<CqSurface> >& aSplits )
{
	TqInt cSplits = 0;

	if ( fPatchMesh() && ( cuSegments() > 1 || cvSegments() > 1 ) )
	{
		std::vector<boost::shared_ptr<CqSurfaceNURBS> > S;

		SubdivideSegments( S );
		TqUint i;
		for ( i = 0; i < S.size(); i++ )
		{
			S[ i ] ->SetSurfaceParameters( *this );
			S[ i ] ->TrimLoops() = TrimLoops();
			S[ i ] ->m_fDiceable = true;
			S[ i ] ->m_SplitDir = m_SplitDir;
			S[ i ] ->SetSplitCount( SplitCount() + 1 );
			aSplits.push_back( S[ i ] );
		}
		return ( i );
	}

	// Split the surface in u or v
	boost::shared_ptr<CqSurfaceNURBS> pNew1( new CqSurfaceNURBS() );
	boost::shared_ptr<CqSurfaceNURBS> pNew2( new CqSurfaceNURBS() );

	// If this primitive is being split because it spans the e and hither planes, then
	// we should split in both directions to ensure we overcome the crossing.
	SplitNURBS( *pNew1, *pNew2, m_SplitDir == SplitDir_U || !m_fDiceable );

	std::vector<CqParameter*>::iterator iUP;
	for ( iUP = m_aUserParams.begin(); iUP != m_aUserParams.end(); iUP++ )
	{
		if ( ( *iUP ) ->Class() != class_vertex )
		{
			CqParameter * pNewA = ( *iUP ) ->Clone();
			CqParameter* pNewB = ( *iUP ) ->Clone();
			( *iUP ) ->Subdivide( pNewA, pNewB, SplitDir() == SplitDir_U, this );
			pNew1->AddPrimitiveVariable( pNewA );
			pNew2->AddPrimitiveVariable( pNewB );
		}
	}

	pNew1->SetSurfaceParameters( *this );
	pNew2->SetSurfaceParameters( *this );
	pNew1->TrimLoops() = TrimLoops();
	pNew2->TrimLoops() = TrimLoops();
	pNew1->m_fDiceable = true;
	pNew2->m_fDiceable = true;
	pNew1->m_SplitDir = ( m_SplitDir == SplitDir_U )? SplitDir_V : SplitDir_U;
	pNew2->m_SplitDir = ( m_SplitDir == SplitDir_U )? SplitDir_V : SplitDir_U;
	pNew1->SetSplitCount( SplitCount() + 1 );
	pNew2->SetSplitCount( SplitCount() + 1 );
	pNew1->SetfPatchMesh( false );
	pNew2->SetfPatchMesh( false );

	aSplits.push_back( pNew1 );
	aSplits.push_back( pNew2 );

	cSplits = 2;

	if ( !m_fDiceable )
	{
		std::vector<boost::shared_ptr<CqSurface> > aSplits0;
		std::vector<boost::shared_ptr<CqSurface> > aSplits1;

		cSplits = aSplits[ 0 ] ->Split( aSplits0 );
		cSplits += aSplits[ 1 ] ->Split( aSplits1 );

		aSplits.clear();
		aSplits.swap( aSplits0 );
		aSplits.insert( aSplits.end(), aSplits1.begin(), aSplits1.end() );
	}

	return ( cSplits );
}

//---------------------------------------------------------------------
/** Return whether or not the patch is diceable
 */

bool	CqSurfaceNURBS::Diceable(const CqMatrix& matCtoR)
{
	assert( NULL != P() );

	// If the cull check showed that the primitive cannot be diced due to crossing the e and hither planes,
	// then we can return immediately.
	if ( !m_fDiceable )
		return ( false );

	// Otherwise we should continue to try to find the most advantageous split direction, OR the dice size.
	// Convert the control hull to raster space.
	CqVector3D * avecHull = new CqVector3D[ m_cuVerts * m_cvVerts ];
	TqUint i;

	TqFloat gs = 16.0f;
	const TqFloat* poptGridSize = QGetRenderContext() ->poptCurrent()->GetFloatOption( "System", "SqrtGridSize" );
	if( NULL != poptGridSize )
		gs = poptGridSize[0];

	TqFloat gridsize = 1.0;

	if (gs >= 1.0)
		gridsize = gs * gs;

	for ( i = 0; i < m_cuVerts*m_cvVerts; i++ )
		avecHull[i] = vectorCast<CqVector3D>(matCtoR * P()->pValue(i)[0]);

	// Now work out the longest continuous line in raster space for u and v.
	TqFloat uLen = 0;
	TqFloat vLen = 0;
	TqFloat MaxuLen = 0;
	TqFloat MaxvLen = 0;

	TqUint v;
	for ( v = 0; v < m_cvVerts; v++ )
	{
		TqUint u;
		for ( u = 0; u < m_cuVerts - 1; u++ )
			uLen += (avecHull[ ( v * m_cuVerts ) + u + 1 ] - avecHull[ ( v * m_cuVerts ) + u ]).Magnitude();
		if ( uLen > MaxuLen )
			MaxuLen = uLen;
		uLen = 0;
	}

	TqUint u;
	for ( u = 0; u < m_cuVerts; u++ )
	{
		for ( v = 0; v < m_cvVerts - 1; v++ )
			vLen += (avecHull[ ( ( v + 1 ) * m_cuVerts ) + u ] - avecHull[ ( v * m_cuVerts ) + u ]).Magnitude();
		if ( vLen > MaxvLen )
			MaxvLen = vLen;
		vLen = 0;
	}

	if ( MaxvLen > gridsize || MaxuLen > gridsize )
	{
		m_SplitDir = ( MaxuLen > MaxvLen ) ? SplitDir_U : SplitDir_V;
		delete[] ( avecHull );
		return ( false );
	}

	TqFloat sqrtShadingRate = sqrt(AdjustedShadingRate());
	MaxuLen /= sqrtShadingRate;
	MaxvLen /= sqrtShadingRate;
	m_uDiceSize = max<TqInt>(lround(MaxuLen), 1);
	m_vDiceSize = max<TqInt>(lround(MaxvLen), 1);

	// Ensure power of 2 to avoid cracking
	const TqInt *binary = pAttributes() ->GetIntegerAttribute( "dice", "binary" );
	if ( binary && *binary)
	{
		m_uDiceSize = ceilPow2( m_uDiceSize );
		m_vDiceSize = ceilPow2( m_vDiceSize );
	}

	if ( MaxuLen < FLT_EPSILON || MaxvLen < FLT_EPSILON )
	{
		m_fDiscard = true;
		delete[] ( avecHull );
		return ( false );
	}


	delete[] ( avecHull );
	m_SplitDir = ( MaxuLen > MaxvLen ) ? SplitDir_U : SplitDir_V;

	if ( m_uDiceSize > gs)
		return false;
	if ( m_vDiceSize > gs)
		return false;

	return ( true );
}


//---------------------------------------------------------------------
/** Determine the segment count for the specified trim curve to make each segment the appropriate size
 *  for the current shading rate.
 */

TqInt	CqSurfaceNURBS::TrimDecimation( const CqTrimCurve& Curve )
{
	TqFloat Len = 0;
	TqFloat MaxLen = 0;
	TqInt cSegments = 0;
	CqMatrix matCtoR;
	QGetRenderContext() ->matSpaceToSpace( "camera", "raster", NULL, pTransform().get(), QGetRenderContext()->Time(), matCtoR );

	TqUint iTrimCurvePoint;
	for ( iTrimCurvePoint = 0; iTrimCurvePoint < Curve.cVerts() - 1; iTrimCurvePoint++ )
	{
		// Get the u,v of the current point.
		TqFloat u, v;
		CqVector3D vecCP;
		vecCP = Curve.CP( iTrimCurvePoint );
		u = vecCP.x();
		v = vecCP.y();

		// Get the u,v of the next point.
		TqFloat u2, v2;
		vecCP = Curve.CP( iTrimCurvePoint + 1 );
		u2 = vecCP.x();
		v2 = vecCP.y();

		CqVector3D vecP = vectorCast<CqVector3D>(Evaluate( u, v, P() ));
		vecP = matCtoR * vecP;
		CqVector3D vecP2 = vectorCast<CqVector3D>(Evaluate( u2, v2, P() ));
		vecP2 = matCtoR * vecP2;

		Len = ( vecP2 - vecP ).Magnitude();
		if ( Len > MaxLen )
			MaxLen = Len;
		cSegments++;
	}
	MaxLen /= sqrt(AdjustedShadingRate());

	TqInt SplitCount = static_cast<TqUint>( max(MaxLen, 1.0f) );

	return ( SplitCount * cSegments );
}



void CqSurfaceNURBS::OutputMesh()
{
	TqUint Granularity = 30;  // Controls the number of steps in u and v


	std::vector<CqSurfaceNURBS*>	S( 1 );
	S[ 0 ] = this;

	// Save the grid as a .raw file.
	FILE* fp = fopen( "NURBS.RAW", "w" );

	TqUint s;
	for ( s = 0; s < S.size(); s++ )
	{
		fprintf( fp, "Surface_%d\n", s );
		std::vector<std::vector<CqVector3D> > aaPoints( Granularity + 1 );
		TqUint p;
		for ( p = 0; p <= Granularity; p++ )
			aaPoints[ p ].resize( Granularity + 1 );


		// Compute points on curve

		TqUint i;
		for ( i = 0; i <= Granularity; i++ )
		{
			TqFloat v = ( static_cast<TqFloat>( i ) / static_cast<TqFloat>( Granularity ) )
			            * ( S[ s ] ->m_avKnots[ S[ s ] ->m_cvVerts ] - S[ s ] ->m_avKnots[ S[ s ] ->m_vOrder - 1 ] )
			            + S[ s ] ->m_avKnots[ S[ s ] ->m_vOrder - 1 ];

			TqUint j;
			for ( j = 0; j <= Granularity; j++ )
			{
				TqFloat u = ( static_cast<TqFloat>( j ) / static_cast<TqFloat>( Granularity ) )
				            * ( S[ s ] ->m_auKnots[ S[ s ] ->m_cuVerts ] - S[ s ] ->m_auKnots[ S[ s ] ->m_uOrder - 1 ] )
				            + S[ s ] ->m_auKnots[ S[ s ] ->m_uOrder - 1 ];

				aaPoints[ i ][ j ] = vectorCast<CqVector3D>(S[ s ] ->Evaluate( u, v, P() ));
			}
		}


		for ( i = 0; i < Granularity; i++ )
		{
			TqUint j;
			for ( j = 0; j < Granularity; j++ )
			{
				fprintf( fp, "%f %f %f %f %f %f %f %f %f\n",
				         aaPoints[ i ][ j ].x(), aaPoints[ i ][ j ].y(), aaPoints[ i ][ j ].z(),
				         aaPoints[ i + 1 ][ j + 1 ].x(), aaPoints[ i + 1 ][ j + 1 ].y(), aaPoints[ i + 1 ][ j + 1 ].z(),
				         aaPoints[ i + 1 ][ j ].x(), aaPoints[ i + 1 ][ j ].y(), aaPoints[ i + 1 ][ j ].z() );
				fprintf( fp, "%f %f %f %f %f %f %f %f %f\n",
				         aaPoints[ i ][ j ].x(), aaPoints[ i ][ j ].y(), aaPoints[ i ][ j ].z(),
				         aaPoints[ i ][ j + 1 ].x(), aaPoints[ i ][ j + 1 ].y(), aaPoints[ i ][ j + 1 ].z(),
				         aaPoints[ i + 1 ][ j + 1 ].x(), aaPoints[ i + 1 ][ j + 1 ].y(), aaPoints[ i + 1 ][ j + 1 ].z() );
			}
		}
	}
	fclose( fp );
}

void CqSurfaceNURBS::AppendMesh( const char *name, TqInt index )
{
	TqUint Granularity = 10;  // Controls the number of steps in u and v

	// Save the grid as a .raw file.
	FILE* fp = fopen( name, "a" );

	fprintf( fp, "Surface_%d\n", index );
	std::vector<std::vector<CqVector3D> > aaPoints( Granularity + 1 );
	TqUint p;
	for ( p = 0; p <= Granularity; p++ )
		aaPoints[ p ].resize( Granularity + 1 );


	// Compute points on curve

	TqUint i;
	for ( i = 0; i <= Granularity; i++ )
	{
		TqFloat v = ( static_cast<TqFloat>( i ) / static_cast<TqFloat>( Granularity ) )
		            * ( m_avKnots[ m_cvVerts ] - m_avKnots[ m_vOrder - 1 ] )
		            + m_avKnots[ m_vOrder - 1 ];

		TqUint j;
		for ( j = 0; j <= Granularity; j++ )
		{
			TqFloat u = ( static_cast<TqFloat>( j ) / static_cast<TqFloat>( Granularity ) )
			            * ( m_auKnots[ m_cuVerts ] - m_auKnots[ m_uOrder - 1 ] )
			            + m_auKnots[ m_uOrder - 1 ];

			aaPoints[ i ][ j ] = vectorCast<CqVector3D>(Evaluate( u, v, P() ));
		}
	}


	for ( i = 0; i < Granularity; i++ )
	{
		TqUint j;
		for ( j = 0; j < Granularity; j++ )
		{
			fprintf( fp, "%f %f %f %f %f %f %f %f %f\n",
			         aaPoints[ i ][ j ].x(), aaPoints[ i ][ j ].y(), aaPoints[ i ][ j ].z(),
			         aaPoints[ i + 1 ][ j + 1 ].x(), aaPoints[ i + 1 ][ j + 1 ].y(), aaPoints[ i + 1 ][ j + 1 ].z(),
			         aaPoints[ i + 1 ][ j ].x(), aaPoints[ i + 1 ][ j ].y(), aaPoints[ i + 1 ][ j ].z() );
			fprintf( fp, "%f %f %f %f %f %f %f %f %f\n",
			         aaPoints[ i ][ j ].x(), aaPoints[ i ][ j ].y(), aaPoints[ i ][ j ].z(),
			         aaPoints[ i ][ j + 1 ].x(), aaPoints[ i ][ j + 1 ].y(), aaPoints[ i ][ j + 1 ].z(),
			         aaPoints[ i + 1 ][ j + 1 ].x(), aaPoints[ i + 1 ][ j + 1 ].y(), aaPoints[ i + 1 ][ j + 1 ].z() );
		}
	}
	fclose( fp );
}


void CqSurfaceNURBS::Output( const char* name )
{
	TqUint i;

	// Save the grid as a .out file.
	FILE * fp = fopen( name, "w" );
	fputs( "NuPatch ", fp );

	fprintf( fp, "%d ", m_cuVerts );
	fprintf( fp, "%d ", m_uOrder );

	fputs( "[\n", fp );
	for ( i = 0; i < m_auKnots.size(); i++ )
		fprintf( fp, "%f \n", m_auKnots[ i ] );
	fputs( "]\n", fp );
	fprintf( fp, "%f %f ", 0.0f, 1.0f );

	fprintf( fp, "%d ", m_cvVerts );
	fprintf( fp, "%d ", m_vOrder );

	fputs( "[\n", fp );
	for ( i = 0; i < m_avKnots.size(); i++ )
		fprintf( fp, "%f \n", m_avKnots[ i ] );
	fputs( "]\n", fp );
	fprintf( fp, "%f %f ", 0.0f, 1.0f );

	fputs( "\"Pw\" [\n", fp );
	for ( i = 0; i < P() ->Size(); i++ )
		fprintf( fp, "%f %f %f %f \n", P()->pValue( i )[0].x(), P()->pValue( i )[0].y(), P()->pValue( i )[0].z(), P()->pValue( i )[0].h() );
	fputs( "]\n", fp );

	fclose( fp );
}


void CqSurfaceNURBS::SetDefaultPrimitiveVariables( bool bUseDef_st )
{
	TqInt bUses = Uses();

	if ( USES( bUses, EnvVars_u ) )
	{
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "u" ) );
		u() ->SetSize( cVarying() );

		TqFloat uinc = ( m_umax - m_umin ) / ( cuSegments() );

		TqInt c, r;
		TqInt i = 0;
		for ( c = 0; c < cvSegments() + 1; c++ )
		{
			TqFloat uval = m_umin;
			for ( r = 0; r < cuSegments() + 1; r++ )
			{
				u() ->pValue() [ i++ ] = uval;
				uval += uinc;
			}
		}
	}

	if ( USES( bUses, EnvVars_v ) )
	{
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "v" ) );
		v() ->SetSize( cVarying() );

		TqFloat vinc = ( m_vmax - m_vmin ) / ( cvSegments() );
		TqFloat vval = m_vmin;

		TqInt c, r;
		TqInt i = 0;
		for ( c = 0; c < cvSegments() + 1; c++ )
		{
			for ( r = 0; r < cuSegments() + 1; r++ )
				v() ->pValue() [ i++ ] = vval;
			vval += vinc;
		}
	}

	const TqFloat* pTC = pAttributes() ->GetFloatAttribute( "System", "TextureCoordinates" );
	CqVector2D st1( pTC[ 0 ], pTC[ 1 ] );
	CqVector2D st2( pTC[ 2 ], pTC[ 3 ] );
	CqVector2D st3( pTC[ 4 ], pTC[ 5 ] );
	CqVector2D st4( pTC[ 6 ], pTC[ 7 ] );

	if ( USES( bUses, EnvVars_s ) && !bHasVar(EnvVars_s) && bUseDef_st )
	{
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "s" ) );
		s() ->SetSize( cVarying() );

		TqInt c, r;
		TqInt i = 0;
		for ( c = 0; c <= cvSegments(); c++ )
		{
			TqFloat v = ( 1.0f / ( cvSegments() ) ) * c;
			for ( r = 0; r <= cuSegments(); r++ )
			{
				TqFloat u = ( 1.0f / ( cuSegments() ) ) * r;
				s() ->pValue() [ i++ ] = BilinearEvaluate( st1.x(), st2.x(), st3.x(), st4.x(), u, v );
			}
		}
	}

	if ( USES( bUses, EnvVars_t ) && !bHasVar(EnvVars_t) && bUseDef_st )
	{
		AddPrimitiveVariable( new CqParameterTypedVarying<TqFloat, type_float, TqFloat>( "t" ) );
		t() ->SetSize( cVarying() );

		TqInt c, r;
		TqInt i = 0;
		for ( c = 0; c <= cvSegments(); c++ )
		{
			TqFloat v = ( 1.0f / ( cvSegments() ) ) * c;
			for ( r = 0; r <= cuSegments(); r++ )
			{
				TqFloat u = ( 1.0f / ( cuSegments() ) ) * r;
				t() ->pValue() [ i++ ] = BilinearEvaluate( st1.y(), st2.y(), st3.y(), st4.y(), u, v );
			}
		}
	}

}


/** Split the NURBS surface into B-Spline (sub) surfaces
 */

void CqSurfaceNURBS::SubdivideSegments( std::vector<boost::shared_ptr<CqSurfaceNURBS> >& S )
{
	TqInt uSplits = cuSegments();
	TqInt vSplits = cvSegments();

	// Resize the array to hold the aplit surfaces.
	S.resize( uSplits * vSplits );

	TqInt iu, iv;

	// An array to hold the split points in u and v, fill in the first one for us.
	std::vector<TqInt> uSplitPoint( uSplits + 1 ), vSplitPoint( vSplits + 1 );
	uSplitPoint[ 0 ] = vSplitPoint[ 0 ] = 0;

	// Refine the knot vectors as appropriate to generate the required split points in u
	for ( iu = 1; iu < uSplits; iu++ )
	{
		TqFloat su = ( static_cast<TqFloat>( iu ) / static_cast<TqFloat>( uSplits ) )
		             * ( m_auKnots[ m_cuVerts ] - m_auKnots[ m_uOrder - 1 ] )
		             + m_auKnots[ m_uOrder - 1 ];

		TqUint extra = 0L;
		TqUint last = m_cuVerts + m_uOrder - 1;
		TqFloat midVal = su;
		TqUint middex = FindSpanU( midVal );

		// Search forward and backward to see if multiple knot is already there
		TqUint i = 0;
		TqUint same = 0L;
		if ( auKnots() [ middex ] == midVal )
		{
			i = middex + 1L;
			same = 1L;
			while ( ( i < last ) && ( auKnots() [ i ] == midVal ) )
			{
				i++;
				same++;
			}

			i = middex - 1L;
			while ( ( i > 0L ) && ( auKnots() [ i ] == midVal ) )
			{
				i--;
				middex--;	// middex is start of multiple knot
				same++;
			}
		}

		if ( i <= 0L )           	    // No knot in middle, must create it
		{
			middex = 0;
			while ( auKnots() [ middex + 1L ] < midVal )
				middex++;
			same = 0L;
		}

		extra = m_uOrder - same;
		std::vector<TqFloat> anewKnots( extra );

		if ( same < m_uOrder )           	    // Must add knots
		{
			for ( i = 0; i < extra; i++ )
				anewKnots[ i ] = midVal;
		}

		uSplitPoint[ iu ] = ( extra < m_uOrder ) ? middex - 1L : middex;
		RefineKnotU( anewKnots );
	}

	// Refine the knot vectors as appropriate to generate the required split points in v
	for ( iv = 1; iv < vSplits; iv++ )
	{
		TqFloat sv = ( static_cast<TqFloat>( iv ) / static_cast<TqFloat>( vSplits ) )
		             * ( m_avKnots[ m_cvVerts ] - m_avKnots[ m_vOrder - 1 ] )
		             + m_avKnots[ m_vOrder - 1 ];

		TqUint extra = 0L;
		TqUint last = m_cvVerts + m_vOrder - 1;
		TqFloat midVal = sv;
		TqUint middex = FindSpanV( midVal );
		// Search forward and backward to see if multiple knot is already there
		TqUint i = 0;
		TqUint same = 0L;
		if ( avKnots() [ middex ] == midVal )
		{
			i = middex + 1L;
			same = 1L;
			while ( ( i < last ) && ( avKnots() [ i ] == midVal ) )
			{
				i++;
				same++;
			}

			i = middex - 1L;
			while ( ( i > 0L ) && ( avKnots() [ i ] == midVal ) )
			{
				i--;
				middex--;	// middex is start of multiple knot
				same++;
			}
		}

		if ( i <= 0L )           	    // No knot in middle, must create it
		{
			middex = 0;
			while ( avKnots() [ middex + 1L ] < midVal )
				middex++;
			same = 0L;
		}

		extra = m_vOrder - same;
		std::vector<TqFloat> anewKnots( extra );

		if ( same < m_vOrder )           	    // Must add knots
		{
			for ( i = 0; i < extra; i++ )
				anewKnots[ i ] = midVal;
		}

		vSplitPoint[ iv ] = ( extra < m_vOrder ) ? middex - 1L : middex;
		RefineKnotV( anewKnots );
	}

	// Fill in the end points for the last split.
	uSplitPoint[ uSplits ] = m_cuVerts - 1;
	vSplitPoint[ vSplits ] = m_cvVerts - 1;

	// Now go over the surface, generating the new patches at the split points in the arrays.
	TqInt uPatch, vPatch;
	// Initialise the offset for the first segment.
	TqInt vOffset = 0;
	for ( vPatch = 0; vPatch < vSplits; vPatch++ )
	{
		// Initialise the offset for the first segment.
		TqInt uOffset = 0;
		// Get the end of the next segment in v.
		TqInt vEnd = vSplitPoint[ vPatch + 1 ];

		// Loop across u rows, filling points and knot vectors.
		for ( uPatch = 0; uPatch < uSplits; uPatch++ )
		{
			TqInt uEnd = uSplitPoint[ uPatch + 1 ];

			// The index of the patch we are working on.
			TqInt iS = ( vPatch * uSplits ) + uPatch;
			S[ iS ] = boost::shared_ptr<CqSurfaceNURBS>( new CqSurfaceNURBS );
			S[ iS ] ->SetfPatchMesh( false );
			// Initialise it to the same orders as us, with the calculated control point densities.
			S[ iS ] ->Init( m_uOrder, m_vOrder, ( uEnd + 1 ) - uOffset, ( vEnd + 1 ) - vOffset );

			// Copy any 'vertex' class user primitive variables.
			TqInt iPu, iPv;
			std::vector<CqParameter*>::iterator iUP;
			for ( iUP = aUserParams().begin(); iUP != aUserParams().end(); iUP++ )
			{
				if ( ( *iUP ) ->Class() == class_vertex )
				{
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( S[ iS ] ->cVertex() );

					for ( iPv = 0; iPv <= vEnd - vOffset; iPv++ )
					{
						TqInt iPIndex = ( ( vOffset + iPv ) * m_cuVerts ) + uOffset;
						for ( iPu = 0; iPu <= uEnd - uOffset; iPu++ )
						{
							TqInt iSP = ( iPv * S[ iS ] ->cuVerts() ) + iPu;
							pNewUP->SetValue( ( *iUP ), iSP, iPIndex++ );
						}
					}
					S[ iS ] ->AddPrimitiveVariable( pNewUP );
				}
			}

			// Copy the knot vectors
			TqUint iuK, ivK;
			for ( iuK = 0; iuK < S[ iS ] ->uOrder() + S[ iS ] ->cuVerts(); iuK++ )
				S[ iS ] ->auKnots() [ iuK ] = auKnots() [ uOffset + iuK ];
			for ( ivK = 0; ivK < S[ iS ] ->vOrder() + S[ iS ] ->cvVerts(); ivK++ )
				S[ iS ] ->avKnots() [ ivK ] = avKnots() [ vOffset + ivK ];

			// Set the offset to just after the end of this segment.
			uOffset = uEnd + 1;
		}
		// Set the offset to just after the end of this segment.
		vOffset = vEnd + 1;
	}

	// Now setup any 'varying', 'uniform' or 'constant' class variables on the segments.
	TqInt irow, icol;
	TqInt nuSegs = uSplits;
	TqInt nvSegs = vSplits;
	for ( icol = 0; icol < nvSegs; icol++ )
	{
		for ( irow = 0; irow < nuSegs; irow++ )
		{
			TqInt iPatch = ( icol * nuSegs ) + irow;
			TqInt iA = ( icol * ( nuSegs + 1 ) ) + irow;
			TqInt iB = ( icol * ( nuSegs + 1 ) ) + irow + 1;
			TqInt iC = ( ( icol + 1 ) * ( nuSegs + 1 ) ) + irow;
			TqInt iD = ( ( icol + 1 ) * ( nuSegs + 1 ) ) + irow + 1;

			std::vector<CqParameter*>::iterator iUP;
			for ( iUP = aUserParams().begin(); iUP != aUserParams().end(); iUP++ )
			{
				if ( ( *iUP ) ->Class() == class_varying )
				{
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( 4 );
					pNewUP->SetValue( ( *iUP ), 0, iA );
					pNewUP->SetValue( ( *iUP ), 1, iB );
					pNewUP->SetValue( ( *iUP ), 2, iC );
					pNewUP->SetValue( ( *iUP ), 3, iD );

					S[ iPatch ] ->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_uniform )
				{
					CqParameter * pNewUP = ( *iUP ) ->CloneType( ( *iUP ) ->strName().c_str(), ( *iUP ) ->Count() );
					pNewUP->SetSize( 1 );
					pNewUP->SetValue( ( *iUP ), 0, iPatch );
					S[ iPatch ] ->AddPrimitiveVariable( pNewUP );
				}
				else if ( ( *iUP ) ->Class() == class_constant )
				{
					CqParameter * pNewUP = ( *iUP ) ->Clone( );
					S[ iPatch ] ->AddPrimitiveVariable( pNewUP );
				}
			}
		}
	}
}



//-------------------------------------------------------

} // namespace Aqsis
