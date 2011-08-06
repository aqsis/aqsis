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

/// \file Statistics helper classes
/// \author Chris Foster chris42f (at) gmail (dot) com
///
/// The idea here is to provide classes which help in the gathering of
/// statistics about program operation.  The stats classes have a bool
/// template parameter which can be set to false when the stat is declared to
/// disable statistic collection at compile time.

#ifndef AQSIS_STATS_H_INCLUDED
#define AQSIS_STATS_H_INCLUDED

#include <algorithm>
#include <iostream>
#include <string>
#include <limits>
#include <vector>

#include <boost/format.hpp>

#include <thread.h>

namespace Aqsis {

//------------------------------------------------------------------------------
/// Class for quick and dirty statistics reporting.
///
/// Example to print the number of calls to a function foo() at the end of
/// execution:
///
///   static StatReporter<int> nFooCalls("number of foo() calls");
///
///   void foo(poly)
///   {
///       nFooCalls++;
///   }
///
template<typename T>
class StatReporter
{
    public:
        StatReporter(const std::string& statName, const T& value = T())
            : m_name(statName),
            m_value(value)
        { }

        /// Get the underlying statistic
        T& stat() { return m_value; }

        ~StatReporter()
        {
            std::cout << m_name << " = " << m_value << std::endl;
        }

    private:
        std::string m_name;
        T m_value;
};


//------------------------------------------------------------------------------
/// Statistic class computing a min, max and average over a set of values.
template<typename T, bool enabled=true>
class MinMaxMeanStat
{
    public:
        MinMaxMeanStat()
            : m_count(0),
            m_sum(0),
            m_min(std::numeric_limits<T>::max()),
            m_max(std::numeric_limits<T>::min()),
            m_scale(1)
        { }

        /// Set a value to scale the final statistic by before reporting.
        void setScale(const T& scale)
        {
            m_scale = scale;
        }

        /// Add a value to the average.
        void operator+=(const T& v)
        {
            if(enabled)
            {
                if(v > m_max)
                    m_max = v;
                if(v < m_min)
                    m_min = v;
                m_sum += v;
                ++m_count;
            }
        }

        /// Merge samples from the given stat into this one.
        void merge(const MinMaxMeanStat& s)
        {
            m_count += s.m_count;
            m_sum += s.m_sum;
            m_max = std::max(m_max, s.m_max);
            m_min = std::min(m_min, s.m_min);
        }

        friend std::ostream& operator<<(std::ostream& out,
                                        const MinMaxMeanStat& s)
        {
            if(enabled)
            {
                if(s.m_count == 0)
                    out << "-";
                else
                    out << s.m_scale * s.m_sum/s.m_count
                        << "  (range [" << s.m_scale * s.m_min
                        << ", " << s.m_scale * s.m_max << "])";
            }
            else
                out << "-";
            return out;
        }

    private:
        long long m_count; ///< Number of samples
        T m_sum;    ///< Sum of samples
        T m_min;    ///< Min of samples
        T m_max;    ///< Max of samples
        T m_scale;  ///< Amount to scale values by
};


//------------------------------------------------------------------------------
/// Compute a histogram over a set of values
template<typename T, bool enabled=true>
class HistogramStat
{
    private:
        typedef long long Counter;

    public:
        HistogramStat(const T& min, const T& max, int nbuckets)
            : m_min(min),
            m_delta(nbuckets/double(max-min)),
            m_scale(1),
            m_buckets(nbuckets+1)
        { }

        /// Set a scaling factor for the incoming values.
        void setScale(const T& scale)
        {
            m_delta *= scale/m_scale;
            m_scale = scale;
        }

        /// Add a value to the histogram
        void operator+=(const T& val)
        {
            if(enabled)
            {
                int offset = ifloor(m_delta*(val - m_min));
                if(offset >= 0 && offset < (int)m_buckets.size())
                    ++m_buckets[offset];
                else
                    ++m_buckets.back();
            }
        }

        /// Merge samples from the given stat into this one.
        void merge(const HistogramStat& s)
        {
            assert(m_scale == s.m_scale);
            assert(m_min == s.m_min);
            assert(m_delta == s.m_delta);
            assert(m_buckets.size() == s.m_buckets.size());
            for(int i = 0; i < (int)m_buckets.size(); ++i)
                m_buckets[i] += s.m_buckets[i];
        }

        friend std::ostream& operator<<(std::ostream& out, const HistogramStat& s)
        {
            if(!enabled)
            {
                out << "-";
                return out;
            }
            Counter bucketMax = 0;
            Counter totSamples = 0;
            int nbuckets = s.m_buckets.size();
            for(int i = 0; i < nbuckets; ++i)
            {
                bucketMax = std::max(bucketMax, s.m_buckets[i]);
                totSamples += s.m_buckets[i];
            }
            if(totSamples == 0)
            {
                bucketMax = 1;
                totSamples = 1;
            }
            double rowLenMult = 50.0/bucketMax;
            for(int i = 0; i < nbuckets; ++i)
            {
                double percent = 100.0*s.m_buckets[i]/totSamples;
                double bucketStart = s.m_min + i/s.m_delta;
                Counter rowLen = Counter(rowLenMult*s.m_buckets[i]);
                out << boost::format("%2.1f | %2.0f%% |") % bucketStart % percent;
                for(int j = 0; j < rowLen; ++j)
                    out << '=';
                if(i < nbuckets-1)
                    out << "\n";
            }
            return out;
        }

    private:
        T m_min;
        double m_delta;
        T m_scale;
        std::vector<Counter> m_buckets;
};


//------------------------------------------------------------------------------
/// Really basic counter stat.
template<bool enabled=true>
class SimpleCounterStat
{
    public:
        SimpleCounterStat() : m_count(0) { }

        void operator++()
        {
            if(enabled)
                ++m_count;
        }

        /// Merge samples from the given stat into this one.
        void merge(const SimpleCounterStat& s)
        {
            m_count += s.m_count;
        }

        friend std::ostream& operator<<(std::ostream& out,
                                        const SimpleCounterStat& s)
        {
            if(enabled)
                out << s.m_count;
            else
                out << "-";
            return out;
        }

        long long value() const { return m_count; }

    private:
        long long m_count;
};


//------------------------------------------------------------------------------
/// Threadsafe counter for pipeline resources.
///
/// Reports number created, max in flight, average in flight.
template<bool enabled=true>
class ResourceCounterStat
{
    public:
        ResourceCounterStat()
        {
            m_current = 0;
            m_sum = 0;
            m_max = 0;
            m_nevents = 0;
            m_ncreated = 0;
        }

        /// Increment current resource count
        //
        // Performance note: it's probably better to use a spinlock here,
        // because the function is so short.  (However, we'd need a portable
        // spinlock implementation.)
        void operator++()
        {
            if(enabled)
            {
                LockGuard lk(m_mutex);
                long long current = ++m_current;
                m_sum += current;
                ++m_nevents;
                ++m_ncreated;
                if(m_max < current)
                    m_max = current;
            }
        }

        /// Decrement current resource count
        void operator--()
        {
            if(enabled)
            {
                LockGuard lk(m_mutex);
                long long current = --m_current;
                m_sum += current;
                ++m_nevents;
            }
        }

        friend std::ostream& operator<<(std::ostream& out,
                                        const ResourceCounterStat& s)
        {
            if(enabled)
            {
                out << s.m_ncreated;
                if(s.m_ncreated > 0)
                {
                    out << " (average " << s.m_sum/s.m_nevents
                        << ", max " << s.m_max << ")";
                }
            }
            else
                out << "-";
            return out;
        }

    private:
        Mutex m_mutex;
        long long m_current;  ///< Current number of resources
        long long m_sum;      ///< Sum of current numbers
        long long m_max;      ///< Max instantaneous resources
        long long m_nevents;  ///< Number of creation/deletion events
        long long m_ncreated; ///< Total number of resources created
};


} // namespace Aqsis
#endif // AQSIS_STATS_H_INCLUDED
