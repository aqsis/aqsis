// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

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
