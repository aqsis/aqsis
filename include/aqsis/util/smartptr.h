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
