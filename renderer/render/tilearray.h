// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
 * \file tilearray.h
 *
 * \brief Declare array class which holds data as tiles and competes with other
 * arrays for memory via a common cache.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef TILE_ARRAY_H_INCLUDED
#define TILE_ARRAY_H_INCLUDED

#include <boost/intrusive_ptr.hpp>

#include "aqsis.h"
#include "memorysentry.h"


namespace Aqsis {

class CqTextureTile; // forward declaration

//------------------------------------------------------------------------------

/** \brief A minimal vector interface to the colour (or other) samples held in
 * a CqTextureTile.
 */
template<typename T>
class CqSampleVector
{
	public:
		/** \brief Construct a sample vector.
		 *
		 * \param data is the raw vector sample data.
		 * \param parentTile is the array tile which the sample data belongs to.
		 */
		inline CqSampleVector(const T* data, boost::intrusive_ptr<CqTextureTile<T> >& parentTile);
		/** \brief Indexing operator for this vector of data.
		 *
		 * \param index is the index at which to obtain a sample.
		 */
		inline T operator[](TqUint index);
	private:
		/// Raw pointer to the data held by this vector
		const T* m_data;
		/** A pointer to the parent tile; this is simply to protect the tile
		 *  data from deallocation in the multithreaded case.
		 */
		boost::intrusive_ptr<CqTextureTile> m_parentTile;
};


//------------------------------------------------------------------------------
/** \brief A class to hold the data from a single texture tile.
 */
template<typename T>
class CqTextureTile
{
	public:
		/** \brief Construct a texture tile
		 */
		CqTextureTile(tdata_t data, );

		/** \brief Index operator - get a vector of samples for this 
		 */
		inline CqSampleVector<T> operator(TqUint i, TqUint j);
	private:
		/// Increase the reference count; required for boost::intrusive_ptr
		friend inline void intrusive_ptr_add_ref(CqTextureTile* ptr);
		/// Decrease the reference count; required for boost::intrusive_ptr
		friend inline void intrusive_ptr_release(CqTextureTile* ptr);

		/// Width of the tile
		TqInt m_width;
		/// Height of the tile
		TqInt m_height;
		/// Number of samples per pixel
		TqInt m_numSamples;
		/// reference count for use with boost::intrusive_ptr
		TqInt m_referenceCount;
		/// pointer to the underlying data
		tdata_t m_data;
};


//------------------------------------------------------------------------------
// TODO: How will CqTextureTileArray<T> be held independently from T??
template<typename T>
class CqTextureTileArray : CqMemoryMonitored
{
	public:
		CqTextureTileArray()

		/** \brief Array indexing operator.
		 *
		 * \param i
		 * \param j
		 * \returns a lightweight vector holding a reference to the sample data
		 */
		inline CqSampleVector<T> operator()(TqUint i, TqUint j);

		static CqTextureTileArray<T> getTileArray(*TIFF tiffFile);

		TqInt width();
		TqInt height();

		/// Inherited from CqMemoryMonitored
		virtual CqMemorySentry::TqMemorySize zapMemory();
	private:
		std::map<CqTextureTile> hotMap;
		std::map<CqTextureTile> hotMap;
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Implementation of inline functions
//------------------------------------------------------------------------------

/** \todo: Threading: the following two functions are not thread-safe.  Using
 * an intrusive pointer is far more efficient/lightweight than a shared_ptr
 * when these two functions have the non-threadsafe implementaion below.  If a
 * threadsafe version turns out not to be very efficient, it might be worth
 * going back to a shared_ptr instead.
 */
inline void intrusive_ptr_add_ref(CqTextureTile* ptr)
{
    ++ptr->m_referenceCount;
}

inline void intrusive_ptr_release(CqTextureTile* ptr)
{
    if(--ptr->m_referenceCount == 0)
        delete ptr;
}

//------------------------------------------------------------------------------
} // namespace Aqsis

#endif // TILE_ARRAY_H_INCLUDED
