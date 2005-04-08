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
		\brief Declares the function TestCondition().
		\author Michel Joron (joron@sympatico.ca)
*/

//? Is .h included already?
#ifndef CONDITION_H_INCLUDED
#define CONDITION_H_INCLUDED 1

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

TqBool TestCondition(RtString condition, RtString where);


//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif // CONDITION_H_INCLUDED

