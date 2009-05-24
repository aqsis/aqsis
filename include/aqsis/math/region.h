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
		\brief Declares a simple 2D region class, similar to the 3D CqBound.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef REGION_H_INCLUDED
#define REGION_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <aqsis/math/vector2d.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqRegion
 * A simple 2D region class.
 */

class CqRegion 
{
	public:
		CqRegion(TqInt xMin = 0, TqInt yMin = 0, TqInt xMax = 0, TqInt yMax = 0);

	TqInt xMin() const;
	TqInt xMax() const;
	TqInt yMin() const;
	TqInt yMax() const;
	TqInt width() const;
	TqInt height() const;
	TqInt area() const;
	CqVector2D diagonal() const;

	private:
		TqInt		m_xMin;
		TqInt		m_yMin;
		TqInt		m_xMax;
		TqInt		m_yMax;
};


inline CqRegion::CqRegion(TqInt xMin, TqInt yMin, TqInt xMax, TqInt yMax) : m_xMin(xMin), m_yMin(yMin), m_xMax(xMax), m_yMax(yMax)
{
}

inline TqInt CqRegion::xMin() const
{
	return m_xMin;
}

inline TqInt CqRegion::xMax() const
{
	return m_xMax;
}

inline TqInt CqRegion::yMin() const
{
	return m_yMin;
}

inline TqInt CqRegion::yMax() const
{
	return m_yMax;
}

inline TqInt CqRegion::width() const
{
	return m_xMax - m_xMin;
}

inline TqInt CqRegion::height() const
{
	return m_yMax - m_yMin;
}

inline TqInt CqRegion::area() const
{
	return width() * height();
}

inline CqVector2D CqRegion::diagonal() const
{
	return CqVector2D(m_xMax - m_xMin, m_yMax - m_yMin);
}
//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !REGION_H_INCLUDED


