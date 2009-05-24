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
 * \brief Define a texture wrap mode enum.
 *
 * \author Chris Foster [chris42f (at) gmail (d0t) com]
 */

#ifndef WRAPMODE_H_INCLUDED
#define WRAPMODE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <string>

#include <aqsis/util/enum.h>

namespace Aqsis
{

//----------------------------------------------------------------------
/** \brief Defines the various modes of handling texture access outside of the
 * normal range.
 */
enum EqWrapMode
{
	WrapMode_Black,		///< Return black.
	WrapMode_Periodic,	///< Wrap around to the opposite side.
	WrapMode_Clamp,		///< Clamp to the edge of the range.
	WrapMode_Trunc		///< Truncate the support (ignores weights)
};

AQSIS_ENUM_INFO_BEGIN(EqWrapMode, WrapMode_Black)
	"black",
	"periodic",
	"clamp",
	"trunc"
AQSIS_ENUM_INFO_END


/// A pair of wrap modes to specify wrapping in the two texture coordinate directions.
struct SqWrapModes
{
	EqWrapMode sWrap;
	EqWrapMode tWrap;
	/// Trivial constructor
	SqWrapModes(EqWrapMode sWrap = WrapMode_Black, EqWrapMode tWrap = WrapMode_Black)
		: sWrap(sWrap), tWrap(tWrap)
	{}
};

} // namespace Aqsis

#endif // WRAPMODE_H_INCLUDED
