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
		\brief Declares the CqVCNoise class for producing noise.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef NOISE_H_INCLUDED
#define NOISE_H_INCLUDED 1

#include	"aqsis.h"

#include	"random.h"
#include	"vector3d.h"
#include	"color.h"

START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqNoise
 * Class implementing noise.
 */

#define	P1x	0.34
#define	P1y	0.66
#define	P1z	0.237

#define	P2x	0.011
#define	P2y	0.845
#define	P2z	0.037

#define	P3x	0.34
#define	P3y	0.12
#define	P3z	0.9



#define NOISE_B 0x100
#define NOISE_BM 0xff

#define NOISE_N 0x1000

class CqNoise
{
public:
    CqNoise()
    {
        init( 665);
    }
    ~CqNoise()
    {}

    static	TqFloat	FGNoise1( TqFloat x );
    static	TqFloat	FGNoise2( TqFloat x, TqFloat y );
    static	TqFloat	FGNoise3( const CqVector3D& v )
    {
	TqFloat a, b, c;
	a = v.x();
	b = v.y();
	c = v.z();
        return ( FGNoise3( a, b, c));
    }
    static	TqFloat	FGNoise3( TqFloat x, TqFloat y, TqFloat z );

    static	CqVector3D	PGNoise1( TqFloat x )
    {
	TqFloat a, b, c;
        a = FGNoise1( x + P1x );
        b = FGNoise1( x + P2x );
        c = FGNoise1( x + P3x );

        return (CqVector3D(a,b,c));
    }
    static	CqVector3D	PGNoise2( TqFloat x, TqFloat y )
    {
	TqFloat a, b, c;
        a = FGNoise2( x + P1x , y + P1y);
        b = FGNoise2( x + P2x , y + P2y);
        c = FGNoise2( x + P3x , y + P3y);

        return (CqVector3D(a,b,c));
    }
    static	CqVector3D	PGNoise3( const CqVector3D& v )
    {
	TqFloat a, b, c;
	a = v.x();
	b = v.y();
	c = v.z();

        return ( PGNoise3( a, b, c));
    }
    static	CqVector3D	PGNoise3( TqFloat x, TqFloat y, TqFloat z )
    {
	TqFloat a,b,c;
        a = FGNoise3( x + P1x, y + P1y, z + P1z );
        b = FGNoise3( x + P2x, y + P2y, z + P2z );
        c = FGNoise3( x + P3x, y + P3y, z + P3z );

        return (CqVector3D (a,b,c));
    }

    static	CqColor	CGNoise1( TqFloat x )
    {
        return ( CGNoise3( x, 0, 0 ) );
    }
    static	CqColor	CGNoise2( TqFloat x, TqFloat y )
    {
        return ( CGNoise3( x, y, 0 ) );
    }
    static	CqColor	CGNoise3( const CqVector3D& v )
    {
	TqFloat a,b,c;
        a = v.x();
        b = v.y();
        c = v.z();

        return ( CGNoise3( a, b, c));
    }
    static	CqColor	CGNoise3( TqFloat x, TqFloat y, TqFloat z )
    {
        return ( CqColor( PGNoise3( x, y, z ) ) );
    }

    static	void	init( TqInt seed );
    static	float	glattice( TqInt ix, TqInt iy, TqInt iz, TqFloat fx, TqFloat fy, TqFloat fz );

private:

    static	TqInt	m_p[ NOISE_B + NOISE_B + 2 ];
    static	TqFloat	m_g3[ NOISE_B + NOISE_B + 2 ][ 3 ];
    static	TqFloat	m_g2[ NOISE_B + NOISE_B + 2 ][ 2 ];
    static	TqFloat	m_g1[ NOISE_B + NOISE_B + 2 ];
};


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !NOISE_H_INCLUDED
