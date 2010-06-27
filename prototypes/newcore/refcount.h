// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
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


#ifndef AQSIS_REFCOUNT_H_INCLUDED
#define AQSIS_REFCOUNT_H_INCLUDED

#include <boost/interprocess/detail/atomic.hpp> // for atomic refcount
#include <boost/intrusive_ptr.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>

inline boost::uint32_t atomic_inc32(volatile boost::uint32_t* i)
{
    return boost::interprocess::detail::atomic_inc32(i);
}
inline boost::uint32_t atomic_dec32(volatile boost::uint32_t* i)
{
    return boost::interprocess::detail::atomic_dec32(i);
}

//------------------------------------------------------------------------------
/// Reference counted base mixin for use with boost::intrusive_ptr.
///
/// This is a non-virtual implementation for maximum efficiency.
class RefCounted
{
    public:
        RefCounted() : m_refCount(0) {}

        /// Copying does *not* copy the reference count!
        RefCounted(const RefCounted& /*r*/) : m_refCount(0) {}
        RefCounted& operator=(const RefCounted& /*r*/) { return *this; }

        int useCount() const { return m_refCount; }
//        int incRef() const   { return ++m_refCount; }
//        int decRef() const   { return --m_refCount; }
        int incRef() const
        {
            return atomic_inc32(&m_refCount) + 1;
        }
        int decRef() const
        {
            return atomic_dec32(&m_refCount) - 1;
        }

    protected:
        /// Protected so users can't delete RefCounted directly.
        ~RefCounted() {}

    private:
//        mutable int m_refCount;
        // todo: is volatile needed here?
        mutable volatile boost::uint32_t m_refCount;
};


/// Add a reference to a RefCounted object.
inline void intrusive_ptr_add_ref(RefCounted* p)
{
    p->incRef();
}

/// Release a reference to a RefCounted object.
///
/// Note that this function *must* be a template, because RefCounted does not
/// have a virtual destructor.  (Therefore, if we just took p as type
/// RefCounted*, the wrong destructor would get called!)
template<typename T>
inline typename boost::enable_if<boost::is_base_of<RefCounted, T> >::type
intrusive_ptr_release(T* p)
{
    if(p->decRef() == 0)
        delete p;
}


//------------------------------------------------------------------------------
/// Null deleter util for use with boost::shared_ptr
inline void nullDeleter(const void*) { }

#endif // AQSIS_REFCOUNT_H_INCLUDED
