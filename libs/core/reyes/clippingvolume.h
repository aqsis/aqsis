// Aqsis
// Copyright (C) 2001, Paul C. Gregory
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
		\brief Implements CqPoints primitives using regular polygon (first try).
		\author M. Joron (joron@sympatico.ca)
*/

//? Is .h included already?
#ifndef CLIPPINGVOLUME_H_INCLUDED
#define CLIPPINGVOLUME_H_INCLUDED

#include	<aqsis/aqsis.h>
#include	<aqsis/math/vector3d.h>
#include	"bound.h"

#include	<vector>

namespace Aqsis {

class CqClippingVolume
{
	public:
		CqClippingVolume()
		{}
		~CqClippingVolume()
		{}


		void addPlane(const CqPlane& plane)
		{
			m_Planes.push_back(plane);
		}

		void clear()
		{
			m_Planes.clear();
		}

		TqInt whereIs(CqBound bound)
		{
			bool bothSides = false;
			std::vector<CqPlane>::iterator i;
			for(i = m_Planes.begin(); i != m_Planes.end(); ++i)
			{
				TqInt side = bound.whichSideOf(&(*i));
				if (side == CqBound::Side_Outside)
					return(CqBound::Side_Outside);
				if (side == CqBound::Side_Both)
					bothSides = true;
			}
			return(bothSides ? CqBound::Side_Both : CqBound::Side_Inside);
		}


	private:
		std::vector<CqPlane>	m_Planes;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !CLIPPINGVOLUME_H_INCLUDED
