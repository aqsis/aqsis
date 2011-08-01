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

/// \file Bucket scheduling
/// \author Chris Foster chris42f (at) gmail (d0t) com

#ifndef AQSIS_BUCKETSCHEDULER_H_INCLUDED
#define AQSIS_BUCKETSCHEDULER_H_INCLUDED

#include "thread.h"
#include "util.h"

namespace Aqsis {

/// Bucket scheduler data shared between threads
class BucketSchedulerShared
{
    public:
        BucketSchedulerShared(V2i nbuckets)
            : nbuckets(nbuckets),
            pos(0)
        { }

    private:
        friend class BucketScheduler;

        V2i nbuckets;
        V2i pos;
        Mutex mutex;
};


/// Scheduler for bucket ordering.
class BucketScheduler
{
    public:
        BucketScheduler(BucketSchedulerShared& shared)
            : m_shared(shared),
            m_pos(0),
            m_begin(0),
            m_end(0)
        { }

        /// Get the next bucket in the ordering.
        ///
        /// For now, this is a simple regular grid ordering.
        ///
        /// This function is threadsafe.
        virtual bool nextBucket(V2i& pos)
        {
            ++m_pos.x;
            if(m_pos.x >= m_end.x)
            {
                ++m_pos.y;
                m_pos.x = m_begin.x;
                if(m_pos.y >= m_end.y)
                {
                    {
                        LockGuard lk(m_shared.mutex);
                        m_begin = m_shared.pos;
                        m_shared.pos.x += m_blockSize;
                        if(m_shared.pos.x >= m_shared.nbuckets.x)
                        {
                            m_shared.pos.x = 0;
                            m_shared.pos.y += m_blockSize;
                        }
                    }
                    m_pos = m_begin;
                    m_end = min(m_begin + V2i(m_blockSize),
                                m_shared.nbuckets);
                }
            }
            if(m_pos.y >= m_shared.nbuckets.y)
                return false;
            pos = m_pos;
            return true;
        }

    private:
        static const int m_blockSize = 2;
        BucketSchedulerShared& m_shared;
        V2i m_pos;
        V2i m_begin;
        V2i m_end;
        Mutex m_mutex;
};

} // namespace Aqsis

#endif // AQSIS_BUCKETSCHEDULER_H_INCLUDED
