// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief Utilities for dealing with filter supports, including wrapping the
 * support using some wrap mode.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef FILTERSUPPORT_H_INCLUDED
#define FILTERSUPPORT_H_INCLUDED

#include "aqsis.h"

#include "aqsismath.h"

namespace Aqsis
{

struct SqFilterSupport1D
{
	TqInt start;
	TqInt end;
	/// Trivial constructor
	inline SqFilterSupport1D(TqInt start, TqInt end);
	/** \brief Truncate the support so that start is >= 0 and end is <=
	 * length.
	 *
	 * Note that this may result in empty supports.
	 */
	inline void truncate(TqInt rangeStart, TqInt rangeEnd);
	/// Return true if the support is empty.
	inline bool isEmpty() const;
	/// Return true if the support covers part of the given range.
	inline bool intersectsRange(TqInt rangeStart, TqInt rangeEnd) const;
	/// Return true if the support is wholly inside the given range.
	inline bool inRange(TqInt rangeStart, TqInt rangeEnd) const;
};


/** \brief Hold filter support area.
 *
 * The end markers are *exclusive*, so the support of a filter is inside the
 * rectangular region [sx.start, ..., endX-1] x [startY, ..., endY-1].
 */
struct SqFilterSupport
{
	SqFilterSupport1D sx; ///< support in x-direction
	SqFilterSupport1D sy; ///< support in y-direction
	/// Trivial constructor.
	inline SqFilterSupport(TqInt startX = 0, TqInt endX = 0, TqInt startY = 0, TqInt endY = 0);
	/// Return true if the support is an empty set.
	inline bool isEmpty() const;
	inline bool inRange(TqInt startX, TqInt endX, TqInt startY, TqInt endY) const;
};



//==============================================================================
// Implementation details
//==============================================================================
// SqFilterSupport1D

inline SqFilterSupport1D::SqFilterSupport1D(TqInt start, TqInt end)
	: start(start), end(end)
{ }

inline void SqFilterSupport1D::truncate(TqInt rangeStart, TqInt rangeEnd)
{
	if(start < rangeStart)
		start = rangeStart;
	if(end > rangeEnd)
		end = rangeEnd;
}

inline bool SqFilterSupport1D::isEmpty() const
{
	return start >= end;
}

inline bool SqFilterSupport1D::intersectsRange(
		TqInt rangeStart, TqInt rangeEnd) const
{
	return end > rangeStart && start < rangeEnd;
}

inline bool SqFilterSupport1D::inRange(TqInt rangeStart, TqInt rangeEnd) const
{
	return start >= rangeStart && end <= rangeEnd;
}


//------------------------------------------------------------------------------
// SqFilterSupport

inline SqFilterSupport::SqFilterSupport(TqInt startX, TqInt endX,
		TqInt startY, TqInt endY)
	: sx(startX, endX),
	sy(startY, endY)
{ }

inline bool SqFilterSupport::isEmpty() const
{
	return sx.isEmpty() || sy.isEmpty();
}

inline bool SqFilterSupport::inRange(
		TqInt startX, TqInt endX, TqInt startY, TqInt endY) const
{
	return sx.inRange(startX, endX) && sy.inRange(startY, endY);
}

} // namespace Aqsis

#endif // FILTERSUPPORT_H_INCLUDED
