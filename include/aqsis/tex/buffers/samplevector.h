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

/**
 * \file
 *
 * \brief Declare a minimal vector interface to raw sample data in a pixel.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef SAMPLEVECTOR_H_INCLUDED
#define SAMPLEVECTOR_H_INCLUDED

#include <aqsis/aqsis.h>

#include <limits>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A minimal vector interface to the pixel data held by another object.
 *
 * This vector converts the data from the underlying pixel format (of type T)
 * into a float by applying scaling such that integer types are mapped onto the
 * range [0,1].
 */
template<typename T>
class CqSampleVector
{
	public:
		/** \brief Construct a sample vector.
		 *
		 * \param data - the raw vector sample data.
		 */
		inline CqSampleVector(const T* sampleData);
		/** \brief Indexing operator for this vector of data.
		 *
		 * If the vector holds integer data, this function normalises the data
		 * by the maximum integer value.  Therefore, for (unsigned) integers the
		 * range returned by operator[] is between 0 and 1.  For floating point
		 * data, no extra normalisation is used.
		 *
		 * \param index - the index at which to obtain a sample.
		 */
		inline TqFloat operator[](TqInt index) const;
	private:
		/// Raw pointer to the data held by this vector
		const T* m_sampleData;
};


//==============================================================================
// Implementation details
//==============================================================================
// CqSampleVector implementation

template<typename T>
inline CqSampleVector<T>::CqSampleVector(const T* sampleData)
	: m_sampleData(sampleData)
{ }

template<typename T>
inline TqFloat CqSampleVector<T>::operator[](TqInt index) const
{
	// This function is performance critical since it's inside the inner loop
	// for all texture filtering operations.
	//
	// The if statement here should be optimised away.
	if(std::numeric_limits<T>::is_integer)
	{
		// The following division should also be optimized away.
		const TqFloat scale = 1.0/std::numeric_limits<T>::max();
		return scale*m_sampleData[index];
	}
	else
		return m_sampleData[index];
}

} // namespace Aqsis

#endif // SAMPLEVECTOR_H_INCLUDED
