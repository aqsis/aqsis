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

#include <aqsis/aqsis.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>

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
class CqTimer
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
/** \brief Storage for a set of CqTimer's
 *
 * The set supports retreival of timers by identifier, and manages the lifetime
 * of the timer classes.  Dumping the timer results to a stream in text format
 * is supported via the printTimes() function.
 *
 * EnumClassT is a "class enum" specifying enum label identifiers for the
 * individual timers held by CqTimerSet:
 *
 * \code
 * struct EnumClassT
 * {
 *   enum Enum
 *   {
 *     // ... enumeration constants
 *   };
 *   static const int size = length_of_Enum;
 * };
 * \endcode
 */
template<typename EnumClassT>
class CqTimerSet
{
	public:
		// Set up EnumClassT::size timers.
		CqTimerSet();

		/// Get a timer by name, or create a new one if it doesn't exist.
		CqTimer& getTimer(typename EnumClassT::Enum id);

		/// Dump timing results to the given stream
		void printTimes(std::ostream& ostr) const;

	private:
		static void numThou(std::ostream& ostr, int n);
		static std::string timeToString(double time);

		struct SqTimeSort;
		typedef std::vector<boost::shared_ptr<CqTimer> > TqTimerVec;
		TqTimerVec m_timers;
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
 *   CqScopeTimer scopeTimer(someTimer);
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
template<typename EnumClassT>
inline CqTimerSet<EnumClassT>::CqTimerSet()
	: m_timers(EnumClassT::size)
{
	for(int i = 0; i < EnumClassT::size; ++i)
		m_timers[i].reset(new CqTimer());
}

template<typename EnumClassT>
inline CqTimer& CqTimerSet<EnumClassT>::getTimer(typename EnumClassT::Enum id)
{
	return *m_timers[id];
}

/// Functor for sorting times in decreasing order.
template<typename EnumClassT>
struct CqTimerSet<EnumClassT>::SqTimeSort
{
	typedef std::pair<typename EnumClassT::Enum, const CqTimer*> TqValueType;
	bool operator() (const TqValueType& lhs, const TqValueType& rhs) const
	{
		return lhs.second->totalTime() > rhs.second->totalTime();
	}
};

template<typename EnumClassT>
void CqTimerSet<EnumClassT>::printTimes(std::ostream& ostr) const
{
	ostr << std::setw(65) << std::setfill('-') << "-\n";
	char tStr[100];
	time_t t = time(0);
	std::strftime(tStr, sizeof(tStr), " at %X %#x", localtime(&t));
	ostr << "Timings" << tStr << "\n";
	ostr << std::setw(65) << std::setfill('-') << "-\n";

	// Sort the timers first
	std::vector<std::pair<typename EnumClassT::Enum, const CqTimer*> > sorted;
	for(int i = 0, end = m_timers.size(); i < end; ++i)
	{
		sorted.push_back(std::make_pair(
			static_cast<typename EnumClassT::Enum>(i), m_timers[i].get()));
	}
	std::sort(sorted.begin(), sorted.end(), SqTimeSort());

	// Output times
	for(int i = 0, end = sorted.size(); i < end; ++i)
	{
		const CqTimer* timer = sorted[i].second;
		long numSamps = timer->numSamples();
		if(numSamps > 0)
		{
			ostr << sorted[i].first
				<< " took " << timeToString(timer->totalTime())
				<< "(called ";
			numThou(ostr, numSamps);
			ostr << " time" << ((numSamps > 1) ? "s" : "") << ")\n";
		}
	}
}

/// Format a number with commas as thousands separators
template<typename EnumClassT>
void CqTimerSet<EnumClassT>::numThou(std::ostream& ostr, int n)
{
	if (n > 1000)
	{
		numThou(ostr, n / 1000);
		ostr << "," << std::setw(3) << std::setfill('0') << (n - ((n / 1000) * 1000));
	}
	else
		ostr << n;
}

/// Format time as a string, choosing the most appropriate SI prefix
template<typename EnumClassT>
std::string CqTimerSet<EnumClassT>::timeToString(double time)
{
	std::ostringstream ostr;
	ostr.setf(std::ios_base::fixed, std::ios_base::floatfield);
	ostr.precision(2);

	if (time > 0.5)
		ostr << time << " seconds ";
	else if(time > 1e-3)
		ostr << time*1e3 << " milli secs ";
	else
		ostr << time*1e6 << " micro secs ";

	return ostr.str();
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
