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
		\brief Implements the basic raytracing subsystem interface class.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>
#include	"raytrace.h"

namespace Aqsis {


/// Required function that implements Class Factory design pattern for Raytrace libraries
IqRaytrace* CreateRaytracer()
{
	return new CqRaytrace;
}


void CqRaytrace::Initialise()
{}

void CqRaytrace::AddPrimitive(const boost::shared_ptr<IqSurface>& pSurface)
{}

void CqRaytrace::Finalise()
{}


//---------------------------------------------------------------------

} // namespace Aqsis
