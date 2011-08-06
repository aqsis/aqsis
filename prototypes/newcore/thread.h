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

/// \file Utilities for threading
/// \author Chris Foster  chris42f (at) gmail (d0t) com

#ifndef AQSIS_THREAD_H_INCLUDED
#define AQSIS_THREAD_H_INCLUDED

#include <boost/thread.hpp>

// While we're waiting for std::atomic<> to become widespread,
// use the atomic header from Intel's TBB.
//
// Commented out for now, because of portability issues.
//#include "tbb/atomic.h"

namespace Aqsis {

//------------------------------------------------------------------------------
/// Dummy mutex implementation for compiling without threads
///
/// This is a standin for any of the boost::mutex types in the case that we
/// want to disable locking entirely in a single-threaded build.
class NullMutex
{
    public:
        // Lockable methods
        void lock() {}
        bool try_lock() { return true; }
        void unlock() {}

        // TimedLockable methods
        bool timed_lock(boost::system_time const& abs_time) { return true; }
        template<typename DurationType> bool timed_lock(DurationType const& rel_time) { return true; }

        // SharedLockable methods
        void lock_shared() {}
        bool try_lock_shared() { return true; }
        bool unlock_shared() { return true; }
        bool timed_lock_shared(boost::system_time const& abs_time) { return true; }

        // UpgradeLockable methods
        void lock_upgrade() {}
        void unlock_upgrade() {}
        void unlock_upgrade_and_lock() {}
        void unlock_upgrade_and_lock_shared() {}
        void unlock_and_lock_upgrade() {}
};


//------------------------------------------------------------------------------
/// A non-threaded replacement for boost::thread_specific_ptr
///
/// This class mirrors the thread_specific_ptr interface, with the intention of
/// replacing it when we've compiled without threading turned on.  The
/// semantics are a bit different - the held object is destroyed when the
/// NullThreadSpecificPtr is destroyed, rather than when the current thread
/// exits.
///
/// The main reason for wanting a replacement like this is to check any
/// threading overheads.
template <typename T>
class NullThreadSpecificPtr
{
    public:
        typedef void (*CleanupFunc)(T*);

        NullThreadSpecificPtr()
            : m_p(0),
            m_cleanupFunc(0)
        { }
        explicit NullThreadSpecificPtr(CleanupFunc f)
            : m_p(0),
            m_cleanupFunc(f)
        { }

        ~NullThreadSpecificPtr()
        {
            reset();
        }

        T* get() const { return m_p; }
        T* operator->() const { return m_p; }
        T& operator*() const { return *m_p; }

        T* release()
        {
            T* oldValue = m_p;
            m_p = 0;
            return oldValue;
        }

        void reset(T* newValue=0)
        {
            if(m_p)
            {
                if(m_cleanupFunc)
                    m_cleanupFunc(m_p);
                else
                    delete m_p;
            }
            m_p = newValue;
        }

    private:
        // Noncopyable
        NullThreadSpecificPtr(const NullThreadSpecificPtr& p);
        NullThreadSpecificPtr& operator=(NullThreadSpecificPtr& p);

        T* m_p;
        CleanupFunc m_cleanupFunc;
};


//------------------------------------------------------------------------------
/// Assign a thread to a fixed cpu.
///
/// If we've got a fixed set of worker threads, equal to the number of cores,
/// it can be useful to pin the threads to the cores, ensuring that the kernel
/// scheduler doesn't move them.  This can result in speedups because it makes
/// more efficient use of the highest levels of the cache heirarchy.  (Each
/// time a process is moved to a different core, the per-core cache becomes
/// invalidated.)
///
/// Note that core 0 sometimes seems to have OS services pinned to it, so it
/// might be best to use other cores first.
///
/// thread - set the affinity of this thread.
/// coreid - pin the thread to this core (0 <= coreid < hardware_concurrency())
///
/// Returns true if successful.
inline bool setThreadAffinity(boost::thread& thread, int coreid)
{
#ifdef AQSIS_USE_THREADS
#ifdef __GNUC__
#ifndef __APPLE__
    // Use pthread_setaffinity_np if using glibc
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreid, &cpuset);
    return !pthread_setaffinity_np(thread.native_handle(), sizeof(cpuset),
                                   &cpuset);
#else
	return true;
#endif
#elif defined(_WIN32)
    // From MSDN... not yet tested!
    return !SetThreadAffinityMask(thread.native_handle(), 1 << coreid);
#else
#   warning "setThreadAffinity() not implemented yet on this platform"
#endif
#else
    return true;
#endif
}


//------------------------------------------------------------------------------
// Handy typedefs
#ifdef AQSIS_USE_THREADS
typedef boost::mutex Mutex;

/// "template typedef" for thread specific pointer
template<typename T>
struct ThreadSpecificPtr { typedef boost::thread_specific_ptr<T> type; };

#else
// When not using threads, make some typedefs which disable threading in a
// non-intrusive way, to avoid a lot of #ifdefs in the rest of the code
typedef NullMutex Mutex;

template<typename T>
struct ThreadSpecificPtr { typedef NullThreadSpecificPtr<T> type; };
#endif


/// Lock types
typedef boost::lock_guard<Mutex> LockGuard;


//------------------------------------------------------------------------------
// Atomic types and operations

#if 0
// Typedefs for atomic types.  These are names from the C++0x standard.
//
// (Unfortunately the details of the TBB interface differ from the upcoming
// standard however.)
typedef tbb::atomic<bool> atomic_flag;
typedef tbb::atomic<int> atomic_int;
typedef tbb::atomic<long long> atomic_llong;


/// Perform the operation a = max(a,x) atomically.
template<typename T>
inline T atomicAssignMax(tbb::atomic<T>& a, T x)
{
    while(true)
    {
        T b = a;
        if(b >= x)
            return b;
        if(b == a.compare_and_swap(x, b))
            return x;
    }
}
#endif

} // namespace Aqsis
#endif // AQSIS_THREAD_H_INCLUDED
