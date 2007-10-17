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
 * \brief Declare a minimal interface to raw sample data.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef SAMPLEVECTOR_H_INCLUDED
#define SAMPLEVECTOR_H_INCLUDED

#include "aqsis.h"

#include <boost/intrusive_ptr.hpp>

#include "smartptr.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief A minimal vector interface to the colour channels held by another object.
 *
 * The object which holds 
 *
 * The idea of this class is to provide the absolute minimal interface to array
 * data managed by another object, but in a way which ensures that the data
 * which this class points to (but does not own) is not deallocated by another
 * thread.
 */
template<typename T>
class CqSampleVector
{
	public:
		/** \brief Construct a sample vector.
		 *
		 * \param data - the raw vector sample data.
		 * \param parentTile - the array tile which the sample data belongs to.
		 */
		inline CqSampleVector(const T* sampleData,
				const boost::intrusive_ptr<const CqIntrusivePtrCounted>& parentTile);
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
		/** A pointer to the parent tile; this is simply to protect the tile
		 *  data from deallocation in the multithreaded case.
		 */
		const boost::intrusive_ptr<const CqIntrusivePtrCounted> m_parentTile;
};


//==============================================================================
// Implementation of inline functions and templates
//==============================================================================

// Inline functions for CqSampleVector

template<typename T>
inline CqSampleVector<T>::CqSampleVector(const T* sampleData,
		const boost::intrusive_ptr<const CqIntrusivePtrCounted>& parentTile)
	: m_sampleData(sampleData),
	m_parentTile(parentTile)
{ }

template<typename T>
inline TqFloat CqSampleVector<T>::operator[](TqInt index) const
{
	// The following if statement should be optimised away at compile time.
	if(std::numeric_limits<T>::is_integer)
		return static_cast<TqFloat>(m_sampleData[index])
			/ std::numeric_limits<T>::max();
	else
		return m_sampleData[index];
}

} // namespace Aqsis

#endif // SAMPLEVECTOR_H_INCLUDED
