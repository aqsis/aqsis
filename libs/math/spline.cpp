// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Implements the CqCubicSpline class for generic spline functionality.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


#include <aqsis/aqsis.h>

#include <aqsis/math/spline.h>


namespace Aqsis {

TqSplineTypes splineTypes = 
{
	{ "bezier", 3, {{  -1.0f,       3.0f,      -3.0f,       1.0f},
					{   3.0f,      -6.0f,       3.0f,       0.0f},
					{  -3.0f,       3.0f,       0.0f,       0.0f},
					{   1.0f,       0.0f,       0.0f,       0.0f}}
	}, 
	{ "bspline", 1, {{ -1.0f/6.0f,  0.5f,      -0.5f,       1.0f/6.0f},
					 {  0.5f,      -1.0f,       0.5f,       0.0f},
					 { -0.5f,       0.0f,	    0.5f,       0.0f},
					 {  1.0f/6.0f,  2.0f/3.0f,  1.0f/6.0f,  0.0f}}
	},
	{ "catmull-rom", 1, {{ -0.5f,       1.5f,      -1.5f,       0.5f},
						 {  1.0f,      -2.5f,       2.0f,      -0.5f},
						 { -0.5f,       0.0f,       0.5f,       0.0f},
						 {  0.0f,       1.0f,       0.0f,       0.0f}}
	},
	{ "hermite", 2, {{  2.0f,       1.0f,      -2.0f,       1.0f},
							{ -3.0f,      -2.0f,       3.0f,      -1.0f},
							{  0.0f,       1.0f,       0.0f,       0.0f},
							{  1.0f,       0.0f,       0.0f,       0.0f}}
	},
	{ "power", 4, {{  1.0f,       0.0f,       0.0f,       0.0f},
						  {  0.0f,       1.0f,       0.0f,       0.0f},
						  {  0.0f,       0.0f,       1.0f,       0.0f},
						  {  0.0f,       0.0f,       0.0f,       1.0f}}
	},
	{ "linear", 1, {{  0.0f,       0.0f,       0.0f,       0.0f},
						   {  0.0f,       0.0f,       0.0f,       0.0f},
						   {  0.0f,      -1.0f,       1.0f,       0.0f},
						   {  0.0f,       1.0f,       0.0f,       0.0f}}
	}
};



} // namespace Aqsis
//---------------------------------------------------------------------
