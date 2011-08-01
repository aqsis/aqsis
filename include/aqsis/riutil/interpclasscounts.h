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
 * \brief Holder for primitive variable interpolation class counts.
 * \author Chris Foster [chris42f (at) g mail (d0t) com]
 */

#ifndef INTERPCLASSCOUNTS_H_INCLUDED
#define INTERPCLASSCOUNTS_H_INCLUDED

#include <aqsis/aqsis.h>

namespace Aqsis
{

//------------------------------------------------------------------------------
/** \brief Holder for interpolation class counts for primitive variables.
 *
 * Arbitrary primitive variables ("primvar"s) can be attached to most RenderMan
 * primitives.  The way that these are interpolated across the surface of the
 * primitive depends on the "interpolation class" of the primvar.  This
 * generally results in a differing number of elements required to specify the
 * various interpolation schemes for a given primitive type.
 *
 * This struct holds a count for each of the interpolation classes.
 */
struct SqInterpClassCounts
{
	TqInt uniform;
	TqInt varying;
	TqInt vertex;
	TqInt facevarying;
	TqInt facevertex;

	/// Default constructor - initialise all counts to zero.
	SqInterpClassCounts();

	/// Trivial constructor
	SqInterpClassCounts(TqInt uniform, TqInt varying, TqInt vertex,
		TqInt facevarying, TqInt facevertex);
};


//==============================================================================
// Implementation details
//==============================================================================

inline SqInterpClassCounts::SqInterpClassCounts()
	: uniform(0),
	varying(0),
	vertex(0),
	facevarying(0),
	facevertex(0)
{ }

inline SqInterpClassCounts::SqInterpClassCounts(TqInt uniform, TqInt varying,
		TqInt vertex, TqInt facevarying, TqInt facevertex)
	: uniform(uniform),
	varying(varying),
	vertex(vertex),
	facevarying(facevarying),
	facevertex(facevertex)
{ }

}

#endif // INTERPCLASSCOUNTS_H_INCLUDED
