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
		\brief Declares the CircleOfConfusion function for computing depth offield effects.
		\author Andrew Bromage (ajb@spamcop.net)
*/

//? Is .h included already?
#ifndef FOCUS_H_INCLUDED
#define FOCUS_H_INCLUDED 1

#include	"aqsis.h"

//#define		_qShareName	CORE
//#include	"share.h"

START_NAMESPACE( Aqsis )


//----------------------------------------------------------------------
/** Compute the radius of the circle of confusion caused by DOF.
  * \param p_dofdata depth of field parameters (see RiDepthOfField() in ri.cpp for details)
  * \param depth screen space z coordinate
  * \return radius of the circle of confusion
 */

inline	TqFloat	CircleOfConfusion(const TqFloat *p_dofdata, TqFloat depth)
{
    return p_dofdata[3] * fabs(1.0f / depth - 1.0f / p_dofdata[2]);
}


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif // FOCUS_H_INCLUDED
