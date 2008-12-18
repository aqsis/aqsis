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

#include "aqsis.h"

#include "vector2d.h"

namespace Aqsis {

//----------------------------------------------------------------------
/** \class CqRegion
 * A simple 2D region class.
 */

class COMMON_SHARE CqRegion 
{
	public:
		CqRegion(TqFloat XMin = 0.0f, TqFloat YMin = 0.0f, TqFloat XMax = 0.0f, TqFloat YMax = 0.0f);
		CqRegion(const CqVector2D& vecMin, const CqVector2D& vecMax);
		~CqRegion() {}

	const CqVector2D& vecMin() const;
	const CqVector2D& vecMax() const;
	CqVector2D vecCross() const;
	const TqFloat width() const;
	const TqFloat height() const;
	const TqFloat area() const;

	private:
		CqVector2D	m_vecMin;
		CqVector2D	m_vecMax;
};


inline CqRegion::CqRegion(TqFloat XMin, TqFloat YMin, TqFloat XMax, TqFloat YMax)
{
	m_vecMin = CqVector2D(XMin, YMin);
	m_vecMax = CqVector2D(XMax, YMax);
}

inline CqRegion::CqRegion(const CqVector2D& vecMin, const CqVector2D& vecMax) : m_vecMin(vecMin), m_vecMax(vecMax)
{
}

inline const CqVector2D& CqRegion::vecMin() const
{
	return m_vecMin;
}

inline const CqVector2D& CqRegion::vecMax() const
{
	return m_vecMax;
}

inline CqVector2D CqRegion::vecCross() const
{
	return( m_vecMax - m_vecMin );
}

inline const TqFloat CqRegion::width() const
{
	return m_vecMax.x() - m_vecMin.x();
}

inline const TqFloat CqRegion::height() const
{
	return m_vecMax.y() - m_vecMin.y();
}

inline const TqFloat CqRegion::area() const
{
	return width() * height();
}

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !REGION_H_INCLUDED


