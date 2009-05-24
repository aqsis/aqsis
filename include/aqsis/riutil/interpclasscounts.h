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
