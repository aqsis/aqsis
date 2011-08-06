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


#ifndef AQSIS_REFCOUNT_H_INCLUDED
#define AQSIS_REFCOUNT_H_INCLUDED

#include <boost/interprocess/detail/atomic.hpp> // for atomic refcount
#include <boost/intrusive_ptr.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>

namespace Aqsis {

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
inline void intrusive_ptr_add_ref(const RefCounted* p)
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
intrusive_ptr_release(const T* p)
{
    if(p->decRef() == 0)
        delete p;
}


//------------------------------------------------------------------------------
/// Null deleter util for use with boost::shared_ptr
inline void nullDeleter(const void*) { }


} // namespace Aqsis

#endif // AQSIS_REFCOUNT_H_INCLUDED
