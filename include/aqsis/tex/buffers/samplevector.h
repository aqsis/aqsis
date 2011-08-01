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
