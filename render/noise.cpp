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

#include	<math.h>

#include	"aqsis.h"
#include	"noise.h"

#include	"ri.h"


START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
// Static data

TqInt		CqNoise::m_Init=TqFalse;
CqRandom	CqNoise::m_random;
float		CqNoise::m_valueTab[TABSIZE];
float		CqNoise::m_gradientTab[TABSIZE*3];
float		CqNoise::m_table[NENTRIES];


//---------------------------------------------------------------------
/** 1D float noise.
 */

TqFloat CqNoise::FGNoise1(TqFloat x)
{
	TqInt ix;
	TqFloat fx0, fx1;
	TqFloat wx;
	TqFloat vx0, vx1;

	ix = FLOOR(x);
	fx0 = x - ix;
	fx1 = fx0 - 1;
	wx = SMOOTHSTEP(fx0);

	vx0 = glattice(ix,0,0,fx0,0,0);
	vx1 = glattice(ix+1,0,0,fx1,0,0);

	return(LERP(wx, vx0, vx1));
}


//---------------------------------------------------------------------
/** 2D float noise.
 */

TqFloat CqNoise::FGNoise2(TqFloat x, TqFloat y)
{
	TqInt ix, iy;
	TqFloat fx0, fx1, fy0, fy1;
	TqFloat wx, wy;
	TqFloat vx0, vx1, vy0, vy1;

	ix = FLOOR(x);
	fx0 = x - ix;
	fx1 = fx0 - 1;
	wx = SMOOTHSTEP(fx0);

	iy = FLOOR(y);
	fy0 = y - iy;
	fy1 = fy0 - 1;
	wy = SMOOTHSTEP(fy0);

	vx0 = glattice(ix,iy,0,fx0,fy0,0);
	vx1 = glattice(ix+1,iy,0,fx1,fy0,0);
	vy0 = LERP(wx, vx0, vx1);
	vx0 = glattice(ix,iy+1,0,fx0,fy1,0);
	vx1 = glattice(ix+1,iy+1,0,fx1,fy1,0);
	vy1 = LERP(wx, vx0, vx1);

	return(LERP(wy, vy0, vy1));
}


//---------------------------------------------------------------------
/** 3D float noise.
 */

TqFloat CqNoise::FGNoise3(TqFloat x, TqFloat y, TqFloat z)
{
	TqInt ix, iy, iz;
	TqFloat fx0, fx1, fy0, fy1, fz0, fz1;
	TqFloat wx, wy, wz;
	TqFloat vx0, vx1, vy0, vy1, vz0, vz1;

	ix = FLOOR(x);
	fx0 = x - ix;
	fx1 = fx0 - 1;
	wx = SMOOTHSTEP(fx0);

	iy = FLOOR(y);
	fy0 = y - iy;
	fy1 = fy0 - 1;
	wy = SMOOTHSTEP(fy0);

	iz = FLOOR(z);
	fz0 = z - iz;
	fz1 = fz0 - 1;
	wz = SMOOTHSTEP(fz0);

	vx0 = glattice(ix,iy,iz,fx0,fy0,fz0);
	vx1 = glattice(ix+1,iy,iz,fx1,fy0,fz0);
	vy0 = LERP(wx, vx0, vx1);
	vx0 = glattice(ix,iy+1,iz,fx0,fy1,fz0);
	vx1 = glattice(ix+1,iy+1,iz,fx1,fy1,fz0);
	vy1 = LERP(wx, vx0, vx1);
	vz0 = LERP(wy, vy0, vy1);

	vx0 = glattice(ix,iy,iz+1,fx0,fy0,fz1);
	vx1 = glattice(ix+1,iy,iz+1,fx1,fy0,fz1);
	vy0 = LERP(wx, vx0, vx1);
	vx0 = glattice(ix,iy+1,iz+1,fx0,fy1,fz1);
	vx1 = glattice(ix+1,iy+1,iz+1,fx1,fy1,fz1);
	vy1 = LERP(wx, vx0, vx1);
	vz1 = LERP(wy, vy0, vy1);

	return(LERP(wz, vz0, vz1));
}


//---------------------------------------------------------------------
/** Initialise the random permutation tables for noise generation.
 * \param seed Random seed to use.
 */

void CqNoise::init(TqInt seed)
{
    m_random.Reseed(seed);

    TqFloat *table = m_valueTab;
    TqInt i;
	for(i=0; i < TABSIZE; i++)
        *table++=1.0-2.0*m_random.RandomFloat();

	// Fill in the value noise table.
	for(i=0; i<NENTRIES; i++)
	{
		TqFloat x=i/(TqFloat)SAMPRATE;
		x=sqrt(x);
        if(x<1)
			m_table[i]=0.5*(2+x*x*(-5+x*3));
		else
			m_table[i]=0.5*(4+x*(-8+x*(5-x)));
    }

	// Fill in the gradien noise table.
    table=m_gradientTab;
    for(i = 0; i < TABSIZE; i++)
	{
		TqFloat z=1.0-2.0*m_random.RandomFloat();
		// r is radius of x,y circle
		TqFloat r=sqrt(1-z*z);
		// theta is angle in (x,y)
		TqFloat theta=2*RI_PI*m_random.RandomFloat();
		*table++=r*cos(theta);
		*table++=r*sin(theta);
		*table++=z;
    }
} 


//---------------------------------------------------------------------
/** Random permutation table lookup.
 */

TqFloat CqNoise::glattice(TqInt ix, TqInt iy, TqInt iz,TqFloat fx, TqFloat fy, TqFloat fz)
{
    TqFloat *g=&m_gradientTab[INDEX(ix,iy,iz)*3];
    return(g[0]*fx + g[1]*fy + g[2]*fz);
}


unsigned char CqNoise::m_perm[TABSIZE] = {
        225,155,210,108,175,199,221,144,203,116, 70,213, 69,158, 33,252,
          5, 82,173,133,222,139,174, 27,  9, 71, 90,246, 75,130, 91,191,
        169,138,  2,151,194,235, 81,  7, 25,113,228,159,205,253,134,142,
        248, 65,224,217, 22,121,229, 63, 89,103, 96,104,156, 17,201,129,
         36,  8,165,110,237,117,231, 56,132,211,152, 20,181,111,239,218,
        170,163, 51,172,157, 47, 80,212,176,250, 87, 49, 99,242,136,189,
        162,115, 44, 43,124, 94,150, 16,141,247, 32, 10,198,223,255, 72,
         53,131, 84, 57,220,197, 58, 50,208, 11,241, 28,  3,192, 62,202,
         18,215,153, 24, 76, 41, 15,179, 39, 46, 55,  6,128,167, 23,188,
        106, 34,187,140,164, 73,112,182,244,195,227, 13, 35, 77,196,185,
         26,200,226,119, 31,123,168,125,249, 68,183,230,177,135,160,180,
         12,  1,243,148,102,166, 38,238,251, 37,240,126, 64, 74,161, 40,
        184,149,171,178,101, 66, 29, 59,146, 61,254,107, 42, 86,154,  4,
        236,232,120, 21,233,209, 45, 98,193,114, 78, 19,206, 14,118,127,
         48, 79,147, 85, 30,207,219, 54, 88,234,190,122, 95, 67,143,109,
        137,214,145, 93, 92,100,245,  0,216,186, 60, 83,105, 97,204, 52
};



END_NAMESPACE(Aqsis)
//---------------------------------------------------------------------
