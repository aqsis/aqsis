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

#include	"specific.h"	// Needed for namespace macros.

#include	"random.h"
#include	"ri.h"
#include	"vector3D.h"
#include	"color.h"

START_NAMESPACE(Aqsis)

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


#define TABSIZE          256
#define TABMASK          (TABSIZE-1)
#define PERM(x)          m_perm[(x)&TABMASK]
#define INDEX(ix,iy,iz)  PERM((ix)+PERM((iy)+PERM(iz)))
#define SAMPRATE 100  /* table entries per unit distance */
#define NENTRIES (4*SAMPRATE+1)
#define SMOOTHSTEP(x)  ((x)*(x)*(3 - 2*(x)))


class CqNoise
{
	public:
						CqNoise()	{
										if(!m_Init)
										{
											init(665);
											m_Init=TqTrue;
										}
									}
						~CqNoise()	{}

	static	TqFloat		FNoise1(TqFloat x)							{return(FNoise3(x,0,0));}
	static	TqFloat		FNoise2(TqFloat x, TqFloat y)				{return(FNoise3(x,y,0));}
	static	TqFloat		FNoise3(const CqVector3D& v)				{return(FNoise3(v.x(),v.y(),v.z()));}
	static	TqFloat		FNoise3(TqFloat x, TqFloat y, TqFloat z);

	static	CqVector3D	PNoise1(TqFloat x)							{return(PNoise3(x,0,0));}
	static	CqVector3D	PNoise2(TqFloat x, TqFloat y)				{return(PNoise3(x,y,0));}
	static	CqVector3D	PNoise3(const CqVector3D& v)				{return(PNoise3(v.x(),v.y(),v.z()));}
	static	CqVector3D	PNoise3(TqFloat x, TqFloat y, TqFloat z)	{
																		CqVector3D res(
																			FNoise3(x+P1x, y+P1y, z+P1z),
																			FNoise3(x+P2x, y+P2y, z+P2z),
																			FNoise3(x+P3x, y+P3y, z+P3z));
																		return(res);
																	}

	static	CqColor		CNoise1(TqFloat x)							{return(CNoise3(x,0,0));}
	static	CqColor		CNoise2(TqFloat x, TqFloat y)				{return(CNoise3(x,y,0));}
	static	CqColor		CNoise3(const CqVector3D& v)				{return(CNoise3(v.x(),v.y(),v.z()));}
	static	CqColor		CNoise3(TqFloat x, TqFloat y, TqFloat z)	{return(CqColor(PNoise3(x,y,z)));}

	static	TqFloat		FGNoise1(TqFloat x);
	static	TqFloat		FGNoise2(TqFloat x, TqFloat y);
	static	TqFloat		FGNoise3(const CqVector3D& v)				{return(FGNoise3(v.x(),v.y(),v.z()));}
	static	TqFloat		FGNoise3(TqFloat x, TqFloat y, TqFloat z);

	static	CqVector3D	PGNoise1(TqFloat x)							{
																		CqVector3D res(
																			FGNoise1(x+P1x),
																			FGNoise1(x+P2x),
																			FGNoise1(x+P3x));
																		return(res);
																	}
	static	CqVector3D	PGNoise2(TqFloat x, TqFloat y)				{
																		CqVector3D res(
																			FGNoise2(x+P1x, y+P1y),
																			FGNoise2(x+P2x, y+P2y),
																			FGNoise2(x+P3x, y+P3y));
																		return(res);
																	}
	static	CqVector3D	PGNoise3(const CqVector3D& v)				{return(PGNoise3(v.x(),v.y(),v.z()));}
	static	CqVector3D	PGNoise3(TqFloat x, TqFloat y, TqFloat z)	{
																		CqVector3D res(
																			FGNoise3(x+P1x, y+P1y, z+P1z),
																			FGNoise3(x+P2x, y+P2y, z+P2z),
																			FGNoise3(x+P3x, y+P3y, z+P3z));
																		return(res);
																	}

	static	CqColor		CGNoise1(TqFloat x)							{return(CGNoise3(x,0,0));}
	static	CqColor		CGNoise2(TqFloat x, TqFloat y)				{return(CGNoise3(x,y,0));}
	static	CqColor		CGNoise3(const CqVector3D& v)				{return(CGNoise3(v.x(),v.y(),v.z()));}
	static	CqColor		CGNoise3(TqFloat x, TqFloat y, TqFloat z)	{return(CqColor(CGNoise3(x,y,z)));}

	static	void		init(TqInt seed);
	static	float		glattice(TqInt ix, TqInt iy, TqInt iz, TqFloat fx, TqFloat fy, TqFloat fz);

	private:

	static	float		m_valueTab[TABSIZE];
	static	float		m_gradientTab[TABSIZE*3];
    static	float		m_table[NENTRIES];
	static	TqInt		m_Init;
	static	CqRandom	m_random;
	static	unsigned char CqNoise::m_perm[TABSIZE];
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !NOISE_H_INCLUDED
