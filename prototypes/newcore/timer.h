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

// Original OIIO license text follows:
/*
  Copyright 2008 Larry Gritz and the other authors and contributors.
  All Rights Reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the software's owners nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  (This is the Modified BSD License)
*/

/// \file Simple timer facilities
/// \author Originally from OIIO, with modifications

#ifndef AQSIS_TIMER_H_INCLUDED
#define AQSIS_TIMER_H_INCLUDED


#ifdef _WIN32
//# include "osdep.h"
#elif defined(__APPLE__)
# include <mach/mach_time.h>
#else
#include <sys/time.h>
#endif

namespace Aqsis {


/// Simple timer class.
///
/// This class allows you to time things, for runtime statistics and the
/// like.  The simplest usage pattern is illustrated by the following
/// example:
///
/// \code
///    Timer mytimer;                // automatically starts upon construction
///    ...do stuff
///    float t = mytimer();          // seconds elapsed since start
///
///    Timer another (false);        // false means don't start ticking yet
///    another.start ();             // start ticking now
///    another.stop ();              // stop ticking
///    another.start ();             // start again where we left off
///    another.stop ();
///    another.reset ();             // reset to zero time again
/// \endcode
///
class Timer {
public:
#ifdef _WIN32
    typedef LARGE_INTEGER value_t;
    // Sheesh, why can't they use a standard type like stdint's int64_t?
#elif defined(__APPLE__)
    typedef unsigned long long value_t;
#else
    typedef struct timeval value_t;
#endif

    /// Constructor -- reset at zero, and start timing unless optional
    /// 'startnow' argument is false.
    explicit Timer (bool startnow=true, bool enabled=true)
        : m_enabled(enabled),
        m_ticking(false),
        m_elapsed(0)
    {
        if (startnow)
            start();
    }

    /// Destructor.
    ///
    ~Timer () { }

    /// Start ticking, or restart if we have stopped.
    ///
    void start () {
        if (m_enabled && !m_ticking) {
            m_starttime = now();
            m_ticking = true;
        }
    }

    /// Stop ticking.  Any elapsed time will be saved even though we
    /// aren't currently ticking.
    double stop () {
        if (m_enabled && m_ticking) {
            value_t n = now();
            m_elapsed += diff (n, m_starttime);
            m_ticking = false;
        }
        return m_elapsed;
    }

    /// Reset at zero and stop ticking.
    ///
    void reset (void) {
        m_elapsed = 0;
        m_ticking = false;
    }

    /// Return the current elapsed time, and reset elapsed time to zero,
    /// but keep the timer going.
    double lap () {
        double r = m_elapsed;
        m_elapsed = 0;
        if (m_ticking) {
            value_t n = now();
            r += diff (n, m_starttime);
            m_starttime = n;
        }
        return m_elapsed;
    }

    /// Operator () returns the elapsed time so far, including both the
    /// currently-ticking clock as well as any previously elapsed time.
    double operator() (void) const {
        return m_elapsed + time_since_start();
    }

    /// Return just the time since we called start(), not any elapsed
    /// time in previous start-stop segments.
    double time_since_start (void) const {
        if (m_ticking) {
            value_t n = now();
            return diff (m_starttime, n);
        } else {
            return 0;
        }
    }

    /// Merge elapsed time from another timer into the current timer.
    void merge (Timer& t)
    {
        m_elapsed += t.m_elapsed;
    }

private:
    bool m_enabled;       ///< Switch to turn off timing
    bool m_ticking;       ///< Are we currently ticking?
    value_t m_starttime;  ///< Time since last call to start()
    double m_elapsed;     ///< Time elapsed BEFORE the current start().

    /// Platform-dependent grab of current time, expressed as value_t.
    ///
    value_t now (void) const {
        value_t n;
#ifdef _WIN32
        QueryPerformanceCounter (&n);   // From MSDN web site
#elif defined(__APPLE__)
        n = mach_absolute_time();
#else
        gettimeofday (&n, 0);
#endif
        return n;
    }

    /// Platform-dependent difference between two times, expressed in
    /// seconds.
    static double diff (const value_t &then, const value_t &now) {
#ifdef _WIN32
        // From MSDN web site
        value_t freq;
        QueryPerformanceFrequency (&freq);
        return (double)(now.QuadPart - then.QuadPart) / (double)freq.QuadPart;
#elif defined(__APPLE__)
        // NOTE(boulos): Both this value and that of the windows
        // counterpart above only need to be calculated once. In
        // Manta, we stored this on the side as a scaling factor but
        // that would require a .cpp file (meaning timer.h can't just
        // be used as a header). It is also claimed that since
        // Leopard, Apple returns 1 for both numer and denom.
        mach_timebase_info_data_t time_info;
        mach_timebase_info(&time_info);
        double seconds_per_tick = (1e-9*static_cast<double>(time_info.numer))/
          static_cast<double>(time_info.denom);
        return (now - then) * seconds_per_tick;
#else
        return fabs ((now.tv_sec  - then.tv_sec) +
                     (now.tv_usec - then.tv_usec) / 1e6);
#endif
    }
};

/// The usual stupid trick to force macro expansion before concatentation
#define AQSIS_TOKEN_CONCAT(a,b) AQSIS_TOKEN_CONCAT_IMPL(a,b)
#define AQSIS_TOKEN_CONCAT_IMPL(a,b) a ## b

/// Time a scope with the given timer.
///
/// This is a convenience macro used to autogenerate a unique name for a
/// ScopeTimer instance.  Example usage:
///
/// Timer myTimer;
///
/// {
///     TIME_SCOPE(myTimer);
///     // ... code to time
/// }
#define TIME_SCOPE(timer) ScopeTimer \
    AQSIS_TOKEN_CONCAT(_timer_scope_guard_,__LINE__)(timer)

/// Turn off a currently running timer inside the given scope
#define DISABLE_TIMER_FOR_SCOPE(timer) ScopeTimerDisable \
    AQSIS_TOKEN_CONCAT(_disable_timer_scope_guard_,__LINE__)(timer)


/// Scope guard class to enable a timer and disable on scope exit
class ScopeTimer
{
    public:
        ScopeTimer(Timer& timer) : m_timer(timer) { m_timer.start(); }
        ~ScopeTimer() { m_timer.stop(); }
    private:
        Timer& m_timer;
};

/// Scope guard class to _disable_ a running timer inside a scope
class ScopeTimerDisable
{
    public:
        ScopeTimerDisable(Timer& timer) : m_timer(timer) { m_timer.stop(); }
        ~ScopeTimerDisable() { m_timer.start(); }
    private:
        Timer& m_timer;
};


} // namespace Aqsis
#endif // AQSIS_TIMER_H_INCLUDED
