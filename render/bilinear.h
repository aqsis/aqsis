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
		\brief Declares template functions for linearly interpolating any class with operator*, - and + support.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef BILINEAR_H_INCLUDED
#define BILINEAR_H_INCLUDED 1

#include	"specific.h"	// Needed for namespace macros.

START_NAMESPACE(Aqsis)

//---------------------------------------------------------------------
/** Bilinearly evalute the four specified values at the specified intervals.
 * \attention The type to be interpolated must support operator+, operator- and operator*.
 * \param A min u min v.
 * \param B max u min v.
 * \param C min u max v.
 * \param D max u max v.
 * \param s the fraction in the u direction.
 * \param t the fraction in the v direction.
 * \return the interpolated value.
 */

template<class T>
T BilinearEvaluate(const T& A, const T& B, const T& C, const T& D, TqFloat s, TqFloat t)
{
	T AB, CD;
	// Work out where the u points are first, then linear interpolate the v value.
	if(s<=0.0)
	{
		AB=A;
		CD=C;
	}
	else
	{
		if(s>=1.0)
		{
			AB=B;
			CD=D;
		}
		else
		{
			AB=static_cast<T>((B-A)*s+A);
			CD=static_cast<T>((D-C)*s+C);
		}
	}

	T R;
	if(t<=0.0)
		R=AB;
	else
	{
		if(t>=1.0)
			R=CD;
		else
			R=static_cast<T>((CD-AB)*t+AB);
	}
	
	return(R);
}


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !BILINEAR_H_INCLUDED
