// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief Declares classes required for the flexible performance timers.
		\author	Paul C. Gregory - originally based on code published on the CodeProject site
				http://www.codeproject.com/debug/multitimer.asp
*/

//? Is color.h included already?
#ifndef MULTITIMER_H_INCLUDED
//{
#define MULTITIMER_H_INCLUDED 1

#ifdef USE_TIMERS

#define TIME_SCOPE(name) CqTimerProxy _tim(timerFactory.getTimer(name));
#define TIME_FUN(fun) {TIME_SCOPE(#fun);fun;}
#define TIME_LINE(line) TIME_FUN(line)

#define TIMER_START(name) timerFactory.getTimer(name)->startTimer(true);
#define TIMER_STOP(name) timerFactory.getTimer(name)->stopTimer();

#define CLEAR_TIMERS timerFactory.clearTimers();
#define GET_TIMER(name) timerFactory.getTimerChecked(name)
#define T2S(time) timerFactory.timeToString(time)

#define TIMER_DUMP(dest, sort) timerFactory.dump(dest, sort);
#define TIMER_DUMP_FILE(file, sort, mode) timerFactory.dump(file, sort, mode);
#define TIMER_DUMP_CSV(file, sort, mode) timerFactory.dumpCsv(file, sort, mode);

#define TIMER_DUMP_SAMPLES(timer, dest) timerFactory.dumpSamples(timer, dest);
#define TIMER_DUMP_SAMPLES_FILE(timer, file, mode) timerFactory.dumpSamples(timer, file, mode);
#define TIMER_DUMP_SAMPLES_CSV(timer, file, mode) timerFactory.dumpSamplesCsv(timer, file, mode);

#endif

enum EqTimerDestType {OUT_MSGBOX, OUT_TRACE, OUT_CONSOLE};
enum EqTimerSortType {SORT_TIMES, SORT_CALLORDER, SORT_NAMES};
enum EqTimerFileMode {FILE_NEW, FILE_APPEND, FILE_PREPEND};

/*
Example usage:
 
#include "multitimer.h"
 
void my_func()
{
	...
	TIME_FUN(somefunction())
	TIME_LINE(somecode)
	...
 
	CLEAR_TIMERS
	...
 
	{
		TIME_SCOPE("some test")
		...
	}
	...
 
	TIMER_START("some manual timer")
	...
	TIMER_STOP("some manual timer")
	...
 
	TIMER_DUMP(OUT_MSGBOX, SORT_CALLORDER)
	TIMER_DUMP_FILE("c:\\times.txt", SORT_TIMES, FILE_APPEND)
	...
 
	boost::shared_ptr<CqHiFreqTimer> test = GET_TIMER("some test");
	printf("%s", T2S(test.getAverageTime()));
	printf("%s", T2S_WITHCYCLES(test.getTotalTime()));
	TIMER_DUMP_SAMPLES(test, OUT_MSGBOX)
	...
 }
*/

#ifdef USE_TIMERS

#include "aqsis.h"

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <boost/shared_ptr.hpp>

#include "exception.h"

namespace Aqsis {

class CqTimerFactory;
COMMON_SHARE extern CqTimerFactory timerFactory;

class COMMON_SHARE CqHiFreqTimerBase
{
	public:
		inline CqHiFreqTimerBase();
		inline virtual ~CqHiFreqTimerBase()
		{}

		typedef std::vector<double> sampleArray;

		inline bool isRunning() const;

		virtual void startTimer(bool manual) = 0;
		virtual void stopTimer() = 0;
		const sampleArray& getSamples() const;
		unsigned int getNumberSamples() const;
		double getTotalTime() const;
		double getAverageTime() const;
		double getMinimumTime() const;
		double getMaximumTime() const;
		int getTimerNo() const;

		static boost::shared_ptr<CqHiFreqTimerBase> createTimer();

	protected:
		int timerNo;
		bool m_running;
		sampleArray m_samples;

		struct SqOverheads
		{
			SqOverheads() : nested(0), starts(0), manualNested(0), manualStarts(0)
			{}

			inline void started(bool manual);
			inline void stopped();
			inline double getOverhead() const;

private:
			int base, nested, starts;
			int manualBase, manualNested, manualStarts;
		}
		ohs;

		struct SqTimingDetails
		{
			void Setup();

			double perFreq;
			int nextTimer;

			int curBase, curManualBase;
			double startstopOverhead, nestedOverhead, manualNestedOverhead;
		};
		static SqTimingDetails timerDetails;
		// Allow friends to look at timerDetails...
		friend class CqTimerFactory;
		friend struct SqOverheads;
};

class CqTimerProxy
{
	public:
		inline CqTimerProxy(boost::shared_ptr<CqHiFreqTimerBase> timer);
		virtual inline ~CqTimerProxy();
		inline bool operator=(CqTimerProxy&);

	protected:
		boost::shared_ptr<CqHiFreqTimerBase> m_timer;
};


class COMMON_SHARE CqTimerFactory
{
	public:
		CqTimerFactory();
		virtual ~CqTimerFactory();

		void clearTimers();
		inline bool isTimerPresent(const std::string& timerName);
		inline boost::shared_ptr<CqHiFreqTimerBase> getTimerChecked(const std::string& timerName);
		inline boost::shared_ptr<CqHiFreqTimerBase> getTimer(const std::string& functionName);
		const char* getTimerName(const boost::shared_ptr<CqHiFreqTimerBase>& timer);
		const char* timeToString(double time);
		void numThou(std::ostream& ostr, int n);
		void dump(const char* file, EqTimerSortType sort, EqTimerFileMode mode = FILE_NEW);
		void dump(EqTimerDestType dest, EqTimerSortType sort);
		void dumpCsv(const char* file, EqTimerSortType sort, EqTimerFileMode mode);
		void dumpSamples(const char* timer, EqTimerDestType dest);
		void dumpSamples(const char* timer, const char* file, EqTimerFileMode mode = FILE_NEW);
		void dumpSamplesCsv(const char* timer, const char* file, EqTimerFileMode mode);
		void dumpSamples(const boost::shared_ptr<CqHiFreqTimerBase>& timer, EqTimerDestType dest);
		void dumpSamples(const boost::shared_ptr<CqHiFreqTimerBase>& timer, const char* file, EqTimerFileMode mode = FILE_NEW);
		void dumpSamplesCsv(const boost::shared_ptr<CqHiFreqTimerBase>& timer, const char* file, EqTimerFileMode mode);

	private:
		std::ostream* getOStream(EqTimerDestType dest);
		void outputStream(std::ostream* outStream, EqTimerDestType dest);
		std::ostream* getOFStream(const char* file, EqTimerFileMode mode);
		void outputFStream(const char* file, std::ostream* outStream, EqTimerFileMode mode);
		void _dump(std::ostream& ostr, EqTimerSortType sort);
		void _dumpCsv(std::ostream& ostr, EqTimerSortType sort);
		void _dumpTimer(std::ostream& ostr, const char* name);
		void _dumpTimerCsv(std::ostream& ostr, const char* name);
		struct SqSorty
		{
			SqSorty(const std::string& s, const boost::shared_ptr<CqHiFreqTimerBase>& t) : str(s), tim(t)
			{}

			std::string str;
			boost::shared_ptr<CqHiFreqTimerBase> tim;
		};
		struct SqTimeSort
		{	// Highest time first
			bool operator() (const SqSorty& m1, const SqSorty& m2)
			{
				return (m1.tim->getTotalTime() > m2.tim->getTotalTime());
			}
		};
		struct SqOrderSort
		{	// Order timers used
			bool operator() (const SqSorty & m1, const SqSorty & m2)
			{
				return (m1.tim->getTimerNo() < m2.tim->getTimerNo());
			}
		};

		typedef std::map<std::string, boost::shared_ptr<CqHiFreqTimerBase> > TqTimerMap;
		TqTimerMap m_map;
		std::string tempStr;
		char thouSep;
};

/* Inline implementations of performance critical functions.
 */
CqHiFreqTimerBase::CqHiFreqTimerBase()
{
	m_running = false;
	timerNo = ++timerDetails.nextTimer;
}


bool CqHiFreqTimerBase::isRunning() const
{
	return m_running;
}

void CqHiFreqTimerBase::SqOverheads::started(bool manual)
{
	if (manual)
	{
		manualStarts++;
		timerDetails.curManualBase++;
	}
	else
	{
		starts++;
		timerDetails.curBase++;
	}
	base = timerDetails.curBase;
	manualBase = timerDetails.curManualBase;
}

void CqHiFreqTimerBase::SqOverheads::stopped()
{
	nested += timerDetails.curBase - base;
	manualNested += timerDetails.curManualBase - manualBase;
}

double CqHiFreqTimerBase::SqOverheads::getOverhead() const
{
	return (starts + manualStarts * 1.4) * timerDetails.startstopOverhead
	   	   + nested * timerDetails.nestedOverhead
		   + manualNested * timerDetails.manualNestedOverhead;
}

CqTimerProxy::CqTimerProxy(boost::shared_ptr<CqHiFreqTimerBase> timer) : m_timer(timer)
{
	m_timer->startTimer(false);
}

CqTimerProxy::~CqTimerProxy()
{
	m_timer->stopTimer();
}

bool CqTimerProxy::operator=(CqTimerProxy&)
{
	return false;
}	// remove compiler warning

bool CqTimerFactory::isTimerPresent(const std::string& timerName)
{
	return (m_map.find(timerName) != m_map.end());
}

boost::shared_ptr<CqHiFreqTimerBase> CqTimerFactory::getTimerChecked(const std::string& timerName)
{
	if (!isTimerPresent(timerName))
	{
		throw(XqInternal("Invalid timer requested", timerName, __FILE__, __LINE__));
	}
	return getTimer(timerName);
}

boost::shared_ptr<CqHiFreqTimerBase> CqTimerFactory::getTimer(const std::string& functionName)
{
	TqTimerMap::iterator pos = m_map.find(functionName);
	if (pos != m_map.end())
	{
		return(pos->second);
	}
	else
	{
		boost::shared_ptr<CqHiFreqTimerBase> newOne(CqHiFreqTimerBase::createTimer());
		m_map.insert(TqTimerMap::value_type(functionName, newOne));

		return(newOne);
	}
}

#else	// Don't use timing code - just dummy defines
	#define TIMER_SETUP

#define TIME_SCOPE(name)
	#define TIME_FUN(fun) fun;
	#define TIME_LINE(line) TIME_FUN(line)

#define TIMER_START(identifier)
	#define TIMER_STOP(identifier)

#define TIMER_DUMP(dest, sort)
	#define TIMER_DUMP_FILE(dest, sort, mode)
	#define TIMER_DUMP_CSV(dest, sort, mode)

#define TIMER_DUMP_SAMPLES(timer, dest)
	#define TIMER_DUMP_SAMPLES_FILE(timer, dest, mode)
	#define TIMER_DUMP_SAMPLES_CSV(timer, file, mode)

#define CLEAR_TIMERS
	#define GET_TIMER(name) (*new CqHiFreqTimer)
	#define T2S(time) "Timing Disabled"

#include <vector>

class CqHiFreqTimer
{
	public:
		typedef std::vector<double> sampleArray;
		const sampleArray& getSamples() const
		{
			return *new sampleArray();
		}
		unsigned int getNumberSamples() const
		{
			return 0;
		}
		double getTotalTime() const
		{
			return 0;
		}
		double getAverageTime() const
		{
			return 0;
		}
		double getMinimumTime() const
		{
			return 0;
		}
		double getMaximumTime() const
		{
			return 0;
		}
};

#endif

} // namespace Aqsis

//}  // End of #ifdef _H_INCLUDED
#endif
