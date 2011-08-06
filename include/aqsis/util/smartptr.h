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
 * \brief Code related to the boost smart pointers
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef SMARTPTR_H_INCLUDED
#define SMARTPTR_H_INCLUDED

#include <aqsis/aqsis.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Null deleter for holding stack-allocated stuff in a boost::shared_ptr
 *
 * Example:
 * // allocate i on the stack:
 * int i;
 * // suppose for some reason we need to access it through a shared_ptr.  We need:
 * boost::shared_ptr<int>(&i, nullDeleter);
 */
inline void nullDeleter(const void*) { }

//------------------------------------------------------------------------------
/** \brief Very simple class providing reference counting machinery via
 * boost::intrusive_ptr.
 *
 * Classes to be counted with an boost::intrusive_ptr should inherit from this class. 
 *
 * WARNING: Think very carefully before allocating this class on the stack,
 * especially without calling intrusive_ptr_add_ref() on it immediately
 * afterward.  Doing so can cause multiple deallocations of the object if it is
 * subsequently handled via an intrusive_ptr.
 */
class CqIntrusivePtrCounted
{
	public:
		/// Get the number of references count for this object
		inline TqUint refCount() const;
	protected:
		/// Construct a reference counted object
		inline CqIntrusivePtrCounted();
		inline virtual ~CqIntrusivePtrCounted();
	private:
		/// Increase the reference count; required for boost::intrusive_ptr
		friend inline void intrusive_ptr_add_ref(const CqIntrusivePtrCounted* ptr);
		/// Decrease the reference count; required for boost::intrusive_ptr
		friend inline void intrusive_ptr_release(const CqIntrusivePtrCounted* ptr);
		/// reference count for use with boost::intrusive_ptr
		mutable TqUint m_refCount;
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Inline functions for CqIntrusivePtrCounted
//
inline CqIntrusivePtrCounted::CqIntrusivePtrCounted()
	: m_refCount(0)
{ }

// pure virtual destructors need an implementation :-/
inline CqIntrusivePtrCounted::~CqIntrusivePtrCounted()
{ }

inline TqUint CqIntrusivePtrCounted::refCount() const
{
	return m_refCount;
}

/** \todo: Threading: the following two functions are not thread-safe.  Using
 * an intrusive pointer is far more efficient/lightweight than a shared_ptr
 * when these two functions have the non-threadsafe implementaion below.  If a
 * threadsafe version turns out not to be very efficient, it might be worth
 * going back to a shared_ptr instead.
 */
inline void intrusive_ptr_add_ref(const CqIntrusivePtrCounted* ptr)
{
    ++ptr->m_refCount;
}

inline void intrusive_ptr_release(const CqIntrusivePtrCounted* ptr)
{
    if(--ptr->m_refCount == 0)
        delete ptr;
}

} // namespace Aqsis
#endif // SMARTPTR_H_INCLUDED
