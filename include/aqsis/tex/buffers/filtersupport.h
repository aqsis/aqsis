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

#include <aqsis/aqsis.h>

#include <aqsis/math/math.h>

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Represent a filter support region in integer raster coordinates.
 *
 * A support is represented by two integers, "start" and "end".  These specify
 * the range [start,end) where the left end is *inclusive* and the right end
 * *exclusive*.
 *
 * \todo Clean up this class for a more uniform interface (make things more uniform?)
 */
struct SqFilterSupport1D
{
	/// Start of the support, inclusive
	TqInt start;
	/// End of the support range, exclusive.
	TqInt end;
	/// Trivial constructor
	SqFilterSupport1D(TqInt start, TqInt end);
	/** \brief Truncate the support into the interval [rangeStart, rangeEnd)
	 *
	 * Note that this may result in empty supports.
	 */
	void truncate(TqInt rangeStart, TqInt rangeEnd);
	/// Return the number of points in the support.
	TqInt range() const;
	/// Return true if the support is empty.
	bool isEmpty() const;
	/// Return true if the support covers part of the given range.
	bool intersectsRange(TqInt rangeStart, TqInt rangeEnd) const;
	/// Return true if the support is wholly inside the given range.
	bool inRange(TqInt rangeStart, TqInt rangeEnd) const;
};

/// Return the intersection of two support regions.
SqFilterSupport1D intersect(const SqFilterSupport1D s1, const SqFilterSupport1D s2);

//------------------------------------------------------------------------------
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
	SqFilterSupport(TqInt startX = 0, TqInt endX = 0, TqInt startY = 0, TqInt endY = 0);
	/// Constructor from two 1D supports
	SqFilterSupport(const SqFilterSupport1D s1, const SqFilterSupport1D s2);
	/// Return area of the support in number of pixels.
	TqInt area() const;
	/// Return true if the support is an empty set.
	bool isEmpty() const;
	/// Return true if the support covers part of the given range.
	bool intersectsRange(TqInt startX, TqInt endX, TqInt startY, TqInt endY) const;
	/// Return true if the support is wholly inside the given range.
	bool inRange(TqInt startX, TqInt endX, TqInt startY, TqInt endY) const;
};

/// Return the intersection of two supports
SqFilterSupport intersect(const SqFilterSupport& s1, const SqFilterSupport& s2);

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

inline TqInt SqFilterSupport1D::range() const
{
	return end - start;
}

inline bool SqFilterSupport1D::isEmpty() const
{
	return start >= end;
}

inline bool SqFilterSupport1D::intersectsRange(
		TqInt rangeStart, TqInt rangeEnd) const
{
	return !isEmpty() && !(rangeStart >= end || rangeEnd <= start);
}

inline bool SqFilterSupport1D::inRange(TqInt rangeStart, TqInt rangeEnd) const
{
	return start >= rangeStart && end <= rangeEnd;
}


inline SqFilterSupport1D intersect(const SqFilterSupport1D s1, const SqFilterSupport1D s2)
{
	return SqFilterSupport1D(max(s1.start, s2.start), min(s1.end, s2.end));
}

//------------------------------------------------------------------------------
// SqFilterSupport

inline SqFilterSupport::SqFilterSupport(TqInt startX, TqInt endX,
		TqInt startY, TqInt endY)
	: sx(startX, endX),
	sy(startY, endY)
{ }

inline SqFilterSupport::SqFilterSupport(const SqFilterSupport1D sx,
		const SqFilterSupport1D sy)
	: sx(sx),
	sy(sy)
{ }

inline TqInt SqFilterSupport::area() const
{
	return sx.range()*sy.range();
}

inline bool SqFilterSupport::isEmpty() const
{
	return sx.isEmpty() || sy.isEmpty();
}

inline bool SqFilterSupport::inRange(
		TqInt startX, TqInt endX, TqInt startY, TqInt endY) const
{
	return sx.inRange(startX, endX) && sy.inRange(startY, endY);
}

inline bool SqFilterSupport::intersectsRange(TqInt startX, TqInt endX,
		TqInt startY, TqInt endY) const
{
	return sx.intersectsRange(startX, endX) && sy.intersectsRange(startY, endY);
}

inline SqFilterSupport intersect(const SqFilterSupport& s1, const SqFilterSupport& s2)
{
	return SqFilterSupport(intersect(s1.sx, s2.sx), intersect(s1.sy, s2.sy));
}

} // namespace Aqsis

#endif // FILTERSUPPORT_H_INCLUDED
