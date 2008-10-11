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

/** \file
 * \brief Declares classes required for the flexible performance timers.
 * \author Paul C. Gregory
 * \author Chris Foster
 *
 * Originally based on code published on the
 * CodeProject site:  http://www.codeproject.com/debug/multitimer.asp
 */

#ifndef MULTITIMER_H_INCLUDED
#define MULTITIMER_H_INCLUDED

#include "aqsis.h"

#include <iosfwd>
#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/timer.hpp>

namespace Aqsis {

//------------------------------------------------------------------------------
/** \brief Simple accumulated timer class.
 *
 * CqTimer supports accumulation of total time for multiple runs of an
 * operation via the start() and stop() methods.  Each sample is accumulated
 * into the total as it is obtained.
 *
 * Support for other types of statistics is easily possible (such as miniumum
 * and maximum times out of the samples, or even an entire histogram) but these
 * should only be added if needed in the future.
 */
class COMMON_SHARE CqTimer
{
	public:
		/// Initialize a timer with 
		CqTimer();

		/// Start the timer
		void start();
		/// Stop the timer; accumulate time since start() was called into the total
		void stop();
		/// Return total time counted by this timer between start() and stop() calls.
		double totalTime() const;
		/// Return average time between start() and stop() calls.
		double averageTime() const;
		/// Return total number of timing samples recorded.
		long numSamples() const;

	private:
		double m_totalTime;    ///< total time
		long m_numSamples;     ///< total number of samples
		boost::timer m_timer;  ///< current timer
};


//------------------------------------------------------------------------------
/** \brief Factory class and storage for a set of CqTimer's
 *
 * The factory supports retreival of timers by name, and manages the lifetime
 * of the timer classes.  Dumping the timer results to a stream in text format
 * is supported via the dump() function.
 */
COMMON_SHARE class CqTimerSet
{
	public:
		/// Get a timer by name, or create a new one if it doesn't exist.
		CqTimer& getTimer(const std::string& timerName);
		/// Delete all timers from the factory
		void clearTimers();

		/// Dump timing results to the given stream
		void dump(std::ostream& ostr);

	private:
		void numThou(std::ostream& ostr, int n);
		std::string timeToString(double time);

		typedef std::map<std::string, boost::shared_ptr<CqTimer> > TqTimerMap;
		TqTimerMap m_map;
};

//------------------------------------------------------------------------------
/** \brief A scope class for starting and automatically stopping a timer.
 *
 * Create one of these to time a scope:
 *
 * CqTimer someTimer;
 *
 * //...
 *
 * {
 *   AQSIS_TIME_SCOPE(someTimer);
 *
 *   // ...
 *
 * } // someTimer is automatically stopped here.
 */
class CqScopeTimer
{
	public:
		CqScopeTimer(CqTimer& timer);
		~CqScopeTimer();
	private:
		CqTimer& m_timer;
};

/// Add time until the end of the current scope to the given timer
#define AQSIS_TIME_SCOPE(timer) CqScopeTimer aq_scope_timer__(timer)



//==============================================================================
// Implementation details
//==============================================================================
// CqTimer implementation
inline CqTimer::CqTimer()
	: m_totalTime(0),
	m_numSamples(0)
{ }

inline void CqTimer::start()
{
	m_timer.restart();
}

inline void CqTimer::stop()
{
	m_totalTime += m_timer.elapsed();
	++m_numSamples;
}

inline double CqTimer::totalTime() const
{
	return m_totalTime;
}

inline double CqTimer::averageTime() const
{
	if(m_numSamples == 0)
		return 0;
	else
		return m_totalTime/m_numSamples;
}

inline long CqTimer::numSamples() const
{
	return m_numSamples;
}


//------------------------------------------------------------------------------
// CqTimerSet implementation
inline CqTimer& CqTimerSet::getTimer(const std::string& timerName)
{
	TqTimerMap::iterator pos = m_map.find(timerName);
	if (pos != m_map.end())
		return *pos->second;
	else
	{
		boost::shared_ptr<CqTimer> newOne(new CqTimer());
		m_map.insert(TqTimerMap::value_type(timerName, newOne));
		return *newOne;
	}
}

inline void CqTimerSet::clearTimers()
{
	m_map.clear();
}

//------------------------------------------------------------------------------
// CqScopeTimer implementation
inline CqScopeTimer::CqScopeTimer(CqTimer& timer)
	: m_timer(timer)
{
	m_timer.start();
}

inline CqScopeTimer::~CqScopeTimer()
{
	m_timer.stop();
}

} // namespace Aqsis

#endif // MULTITIMER_H_INCLUDED
