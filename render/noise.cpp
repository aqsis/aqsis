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
		\brief Implements the CqNoise class for producing noise.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

/* coherent noise function over 1, 2 or 3 dimensions */
/* (copyright Ken Perlin) */

#include	<math.h>

#include	"aqsis.h"
#include	"noise.h"

#include	"ri.h"


START_NAMESPACE( Aqsis )


#define s_curve(t) ( t * t * (3. - 2. * t) )
#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + NOISE_N;\
	b0 = ((int)t) & NOISE_BM;\
	b1 = (b0+1) & NOISE_BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.;


//---------------------------------------------------------------------
// Static data

TqInt	CqNoise::m_Init = TqFalse;
CqRandom	CqNoise::m_random;
TqInt	CqNoise::m_p[ NOISE_B + NOISE_B + 2 ];
TqFloat	CqNoise::m_g3[ NOISE_B + NOISE_B + 2 ][ 3 ];
TqFloat	CqNoise::m_g2[ NOISE_B + NOISE_B + 2 ][ 2 ];
TqFloat	CqNoise::m_g1[ NOISE_B + NOISE_B + 2 ];


static void normalize2( TqFloat v[ 2 ] )
{
	TqFloat s;

	s = sqrt( v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] );
	v[ 0 ] = v[ 0 ] / s;
	v[ 1 ] = v[ 1 ] / s;
}

static void normalize3( TqFloat v[ 3 ] )
{
	TqFloat s;

	s = sqrt( v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ] );
	v[ 0 ] = v[ 0 ] / s;
	v[ 1 ] = v[ 1 ] / s;
	v[ 2 ] = v[ 2 ] / s;
}

//---------------------------------------------------------------------
/** 1D float noise.
 */

TqFloat CqNoise::FGNoise1( TqFloat x )
{
	TqInt bx0, bx1;
	TqFloat rx0, rx1, sx, t, u, v, vec[ 1 ];

	vec[ 0 ] = x;

	setup( 0, bx0, bx1, rx0, rx1 );
	sx = s_curve( rx0 );

	u = rx0 * m_g1[ m_p[ bx0 ] ];
	v = rx1 * m_g1[ m_p[ bx1 ] ];

	return ( LERP( sx, u, v ) );
}


//---------------------------------------------------------------------
/** 2D float noise.
 */

TqFloat CqNoise::FGNoise2( TqFloat x, TqFloat y )
{
	TqInt bx0, bx1, by0, by1, b00, b10, b01, b11;
	TqFloat rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v, vec[ 2 ];
	register int i, j;

	vec[ 0 ] = x;
	vec[ 1 ] = y;

	setup( 0, bx0, bx1, rx0, rx1 );
	setup( 1, by0, by1, ry0, ry1 );

	i = m_p[ bx0 ];
	j = m_p[ bx1 ];

	b00 = m_p[ i + by0 ];
	b10 = m_p[ j + by0 ];
	b01 = m_p[ i + by1 ];
	b11 = m_p[ j + by1 ];

	sx = s_curve( rx0 );
	sy = s_curve( ry0 );

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = m_g2[ b00 ]; u = at2( rx0, ry0 );
	q = m_g2[ b10 ]; v = at2( rx1, ry0 );
	a = LERP( sx, u, v );

	q = m_g2[ b01 ]; u = at2( rx0, ry1 );
	q = m_g2[ b11 ]; v = at2( rx1, ry1 );
	b = LERP( sx, u, v );

	return ( LERP( sy, a, b ) );
}


//---------------------------------------------------------------------
/** 3D float noise.
 */

TqFloat CqNoise::FGNoise3( TqFloat x, TqFloat y, TqFloat z )
{
	TqInt bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	TqFloat rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v, vec[ 3 ];
	register int i, j;

	vec[ 0 ] = x;
	vec[ 1 ] = y;
	vec[ 2 ] = z;

	setup( 0, bx0, bx1, rx0, rx1 );
	setup( 1, by0, by1, ry0, ry1 );
	setup( 2, bz0, bz1, rz0, rz1 );

	i = m_p[ bx0 ];
	j = m_p[ bx1 ];

	b00 = m_p[ i + by0 ];
	b10 = m_p[ j + by0 ];
	b01 = m_p[ i + by1 ];
	b11 = m_p[ j + by1 ];

	t = s_curve( rx0 );
	sy = s_curve( ry0 );
	sz = s_curve( rz0 );

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

	q = m_g3[ b00 + bz0 ]; u = at3( rx0, ry0, rz0 );
	q = m_g3[ b10 + bz0 ]; v = at3( rx1, ry0, rz0 );
	a = LERP( t, u, v );

	q = m_g3[ b01 + bz0 ]; u = at3( rx0, ry1, rz0 );
	q = m_g3[ b11 + bz0 ]; v = at3( rx1, ry1, rz0 );
	b = LERP( t, u, v );

	c = LERP( sy, a, b );

	q = m_g3[ b00 + bz1 ]; u = at3( rx0, ry0, rz1 );
	q = m_g3[ b10 + bz1 ]; v = at3( rx1, ry0, rz1 );
	a = LERP( t, u, v );

	q = m_g3[ b01 + bz1 ]; u = at3( rx0, ry1, rz1 );
	q = m_g3[ b11 + bz1 ]; v = at3( rx1, ry1, rz1 );
	b = LERP( t, u, v );

	d = LERP( sy, a, b );

	return ( LERP( sz, c, d ) );
}


//---------------------------------------------------------------------
/** Initialise the random permutation tables for noise generation.
 * \param seed Random seed to use.
 */

void CqNoise::init( TqInt seed )
{
	m_random.Reseed( seed );

	TqInt i, j, k;

	for ( i = 0; i < NOISE_B; i++ )
	{
		m_p[ i ] = i;
		m_g1[ i ] = static_cast<TqFloat>( ( static_cast<TqInt>( m_random.RandomInt() ) % ( NOISE_B + NOISE_B ) ) - NOISE_B ) / NOISE_B;
		for ( j = 0; j < 2; j++ )
			m_g2[ i ][ j ] = static_cast<TqFloat>( ( static_cast<TqInt>( m_random.RandomInt() ) % ( NOISE_B + NOISE_B ) ) - NOISE_B ) / NOISE_B;
		normalize2( m_g2[ i ] );

		for ( j = 0; j < 3; j++ )
			m_g3[ i ][ j ] = static_cast<TqFloat>( ( static_cast<TqInt>( m_random.RandomInt() ) % ( NOISE_B + NOISE_B ) ) - NOISE_B ) / NOISE_B;
		normalize3( m_g3[ i ] );
	}

	while ( --i )
	{
		k = m_p[ i ];
		m_p[ i ] = m_p[ j = static_cast<TqInt>( m_random.RandomInt() ) % NOISE_B ];
		m_p[ j ] = k;
	}

	for ( i = 0; i < NOISE_B + 2; i++ )
	{
		m_p[ NOISE_B + i ] = m_p[ i ];
		m_g1[ NOISE_B + i ] = m_g1[ i ];
		for ( j = 0; j < 2; j++ )
			m_g2[ NOISE_B + i ][ j ] = m_g2[ i ][ j ];
		for ( j = 0; j < 3; j++ )
			m_g3[ NOISE_B + i ][ j ] = m_g3[ i ][ j ];
	}

	m_Init = 1;
}


END_NAMESPACE( Aqsis )
//---------------------------------------------------------------------
