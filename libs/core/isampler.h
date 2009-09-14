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
		\brief Defines the interface to objects that can provide sample patterns.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef ISAMPLER_H_INCLUDED //{
#define ISAMPLER_H_INCLUDED 1

#include	<aqsis/aqsis.h>

namespace Aqsis {

class CqVector2D;

//------------------------------------------------------------------------------
/** \brief An interface to sample distribution patterns generation.
 *
 * This interface provides sample distribution information for functions in Aqsis
 * that need to sample signals effectively.
 *
 */
class IqSampler
{
	public:
		virtual ~IqSampler() {}
		/** \brief Return a set of 2D sample positions over the default region.
		 *
		 * Return an array of unit sample positions. The coordinate space is a canonical
		 * unit square, centered around the origin, the caller is responsible for
		 * transforming the point into the appropriate coordinate space.
		 * The returned value points to array storage owned by the sampler, the caller
		 * is not responsible for disposing of the array.
		 *
		 * \note Currently this is expected to return a pixels worth of samples.
		 *
		 * \returns - a constant pointer to an array of sample positions.
		 */
		virtual const CqVector2D* get2DSamples() = 0;		
		/** \brief Return a set of 1D sample positions over the specified region.
		 *
		 * Returns a set of 1D values for the number of samples requested.
		 * The returned value points to array storage owned by the sampler, the caller
		 * is not responsible for disposing of the array.
		 *
		 * \note Currently this is expected to return a pixels worth of samples.
		 *
		 * \returns - a constant pointer to an array of sample times.
		 */
		virtual const TqFloat* get1DSamples() = 0;	
		/** \brief Return a set of 1D shuffle offsets over the specified sample range.
		 *
		 *  Returns a set of integer indices between 0 and the number of samples, randomly
		 *  shuffled for jittering array indices.
		 *
		 * \returns - a constant pointer to an array of integer indices.
		 */
		virtual const TqInt* getShuffledIndices() = 0;
};

} // namespace Aqsis

#endif //} ISAMPLER_H_INCLUDED
