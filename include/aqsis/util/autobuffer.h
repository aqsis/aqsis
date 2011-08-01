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
