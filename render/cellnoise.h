// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Declares the CqCellNoise class fro producing cellular noise.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is .h included already?
#ifndef CELLNOISE_H_INCLUDED
#define CELLNOISE_H_INCLUDED 1

#include	"specific.h"	// Needed for namespace macros.

#include	"ri.h"
#include	"vector3d.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqCellNoise
 * Class encapsulating the functionality of the shader cell noise functions.
 */

class CqCellNoise
{
	public:
				// Default constructor.
				CqCellNoise()	{}
				~CqCellNoise()	{}

		TqFloat		FCellNoise1(TqFloat u);
		TqFloat		FCellNoise2(TqFloat u, TqFloat v);
		TqFloat		FCellNoise3(const CqVector3D& P);
		TqFloat		FCellNoise4(const CqVector3D& P, TqFloat v);

		CqVector3D	PCellNoise1(TqFloat u);
		CqVector3D	PCellNoise2(TqFloat u, TqFloat v);
		CqVector3D	PCellNoise3(const CqVector3D& P);
		CqVector3D	PCellNoise4(const CqVector3D& P, TqFloat v);

	private:
		static TqInt	m_PermuteTable[2*2048];		///< static permutation table.
		static TqFloat	m_RandomTable[2048];		///< static random table.
};


//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !CELLNOISE_H_INCLUDED
