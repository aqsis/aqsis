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
 * \brief Declare a variable sized buffer class containing an internal
 * statically-sized array for commonly allocated array sizes.
 *
 * \author Chris Foster [ chris42f (at) gmail (dot) com ]
 */

#ifndef AUTOBUFFER_H_INCLUDED
#define AUTOBUFFER_H_INCLUDED

#include <aqsis/aqsis.h>

#include <boost/scoped_array.hpp>

namespace Aqsis {

/** \brief Variable size buffer class which allocates on the stack if possible.
 *
 * Sometimes we want to allocate an array on the program stack, but the size of
 * that array isn't known at compile time.  Unfortunately there is no way to do
 * this portably in standard C++.
 *
 * However, there is often a rough bound on the amount of space required and we
 * can allocate this expected amount on the stack, and fall back to using the
 * heap if necessary.  The CqAutoBuffer class implements this strategy.
 *
 * This idea has been implemented in other software (undoubtably many times);
 * stlsoft::auto_buffer one such example and the one from which this class
 * takes its name.  Something like this will probably make its way into boost
 * at some stage...
 *
 * Note also that this is a portable approximate replacement for the alloca()
 * function.
 *
 * T - type to hold in the buffer
 * defaultBufSize - Size of the internal buffer allocated with the object
 *                  (allocated on the stack whenever the buffer instance itself
 *                  is).  Any runtime requested sizes which are larger than
 *                  this will result in a heap allocation.
 */
template<typename T, int defaultBufSize>
class CqAutoBuffer
{
	public:
		/** \brief Construct an autobuffer
		 *
		 * \param size - size of the array of T.
		 */
		CqAutoBuffer(TqInt size);

		/** \brief Construct an autobuffer and initialize elements.
		 *
		 * \param size - size of the array of T.
		 * \param defaultVal - value to initialalize the buffer with.
		 */
		CqAutoBuffer(TqInt size, const T& defaultVal);

		/// Array indexing operators
		T& operator[](TqInt i);
		const T& operator[](TqInt i) const;

		/// Get a pointer to the start of the underlying buffer.
		T* get();
		const T* get() const;

		/// Get the buffer size
		TqInt size() const;
	private:
		/// Fixed-size array of the given expected size.
		T m_defaultBuf[defaultBufSize];
		/// Heap-allocated array, used if we need more than defaultBufSize of memory.
		boost::scoped_array<T> m_heapBuf;
		/// Pointer to the buffer in use.
		T* m_buf;
		/// Size of the buffer.
		TqInt m_size;
};



//==============================================================================
// Implementation details
//==============================================================================

template<typename T, int defaultBufSize>
inline CqAutoBuffer<T, defaultBufSize>::CqAutoBuffer(TqInt size)
	: m_heapBuf(size < defaultBufSize ? 0 : new T[size]),
	m_buf(size < defaultBufSize ? m_defaultBuf : m_heapBuf.get()),
	m_size(size)
{ }

template<typename T, int defaultBufSize>
inline CqAutoBuffer<T, defaultBufSize>::CqAutoBuffer(TqInt size, const T& defaultVal)
	: m_heapBuf(size < defaultBufSize ? 0 : new T[size]),
	m_buf(size < defaultBufSize ? m_defaultBuf : m_heapBuf.get()),
	m_size(size)
{
	for(TqInt i = 0; i < size; ++i)
		m_buf[i] = defaultVal;
}

/// Indexing
template<typename T, int defaultBufSize>
inline T& CqAutoBuffer<T, defaultBufSize>::operator[](TqInt i)
{
	return m_buf[i];
}
template<typename T, int defaultBufSize>
inline const T& CqAutoBuffer<T, defaultBufSize>::operator[](TqInt i) const
{
	return m_buf[i];
}

/// Get at the underlying buffer.
template<typename T, int defaultBufSize>
inline T* CqAutoBuffer<T, defaultBufSize>::get()
{
	return m_buf;
}
template<typename T, int defaultBufSize>
inline const T* CqAutoBuffer<T, defaultBufSize>::get() const
{
	return m_buf;
}

template<typename T, int defaultBufSize>
inline TqInt CqAutoBuffer<T, defaultBufSize>::size() const
{
	return m_size;
}


} // namespace Aqsis

#endif // AUTOBUFFER_H_INCLUDED
