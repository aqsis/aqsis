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
		\brief Declares a simple class representing an infinite plane.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef PLANE_H_INCLUDED
#define PLANE_H_INCLUDED

#include	<aqsis/aqsis.h>
#include	<aqsis/math/vector3d.h>

namespace Aqsis {

class CqPlane
{
	public:
		CqPlane(const CqVector3D& point, const CqVector3D& normal)
		{
			m_a = normal.x();
			m_b = normal.y();
			m_c = normal.z();
			m_d = -(normal*point);
		}

		CqPlane(TqFloat a, TqFloat b, TqFloat c, TqFloat d)
		{
			m_a = a;
			m_b = b;
			m_c = c;
			m_d = d;
		}
		~CqPlane()
		{}

		enum EqHalfSpace
		{
		    Space_Negative = -1,
		    Space_On_Plane = 0,
		    Space_Positive = 1,
	};


		EqHalfSpace whichSide(const CqVector3D& p) const
		{
			TqFloat d = m_a*p.x() + m_b*p.y() + m_c*p.z() + m_d;
			if(d < 0)
				return(Space_Negative);
			if(d > 0)
				return(Space_Positive);
			return(Space_On_Plane);
		}

	private:
		TqFloat m_a, m_b, m_c, m_d;
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !PLANE_H_INCLUDED
