#ifndef __MULTI_TIMER_H__
#define __MULTI_TIMER_H__

// Comment out next line to remove timing code
//#define USE_TIMERS
// Comment out next line to remove clock speed code
//#define _CLOCKTICKS

#ifdef USE_TIMERS
	#define TIME_SCOPE(name) CTimerProxy _tim(timerFactory.getTimer(name));
	#define TIME_FUN(fun) {TIME_SCOPE(#fun);fun;}
	#define TIME_LINE(line) TIME_FUN(line)

	#define TIMER_START(name) timerFactory.getTimer(name).startTimer(true);
	#define TIMER_STOP(name) timerFactory.getTimer(name).stopTimer();

	#define CLEAR_TIMERS timerFactory.clearTimers();
	#define GET_TIMER(name) timerFactory.getTimerChecked(name)
	#define T2S(time) timerFactory.timeToString(time)

	#define TIMER_DUMP(dest, sort) timerFactory.dump(dest, sort);
	#define TIMER_DUMP_FILE(file, sort, mode) timerFactory.dump(file, sort, mode);
	#define TIMER_DUMP_CSV(file, sort, mode) timerFactory.dumpCsv(file, sort, mode);

	#define TIMER_DUMP_SAMPLES(timer, dest) timerFactory.dumpSamples(timer, dest);
	#define TIMER_DUMP_SAMPLES_FILE(timer, file, mode) timerFactory.dumpSamples(timer, file, mode);
	#define TIMER_DUMP_SAMPLES_CSV(timer, file, mode) timerFactory.dumpSamplesCsv(timer, file, mode);

	#ifdef _CLOCKTICKS
		#define CLOCK_SPEED timerFactory.getClockSpeed()
		#define T2S_WITHCYCLES(time) timerFactory.timeToStringWithCycles(time)
		#define T2C(time) timerFactory.timeToCycles(time)
	#endif
#endif

enum timerDestType {OUT_MSGBOX, OUT_TRACE, OUT_CONSOLE};
enum timerSortType {SORT_TIMES, SORT_CALLORDER, SORT_NAMES};
enum timerFileMode {FILE_NEW, FILE_APPEND, FILE_PREPEND};

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

	CHiFreqTimer& test = GET_TIMER("some test");
	printf("%s", T2S(test.getAverageTime()));
	printf("%s", T2S_WITHCYCLES(test.getTotalTime()));
	TIMER_DUMP_SAMPLES(test, OUT_MSGBOX)
	...
 }
*/

#ifdef USE_TIMERS

#pragma warning(disable:4786)	// hide stl warnings (VS6)

#include <windows.h>
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

class CTimerFactory;
extern CTimerFactory timerFactory;

class CHiFreqTimer
{
public:
	typedef std::vector<double> sampleArray;

	CHiFreqTimer()
	{
		startTime.QuadPart = 0;
		m_running = false;
		timerNo = ++timerDetails.nextTimer;
	}
	virtual ~CHiFreqTimer()
	{
		if (m_running)
			stopTimer();
	}

	bool isRunning() const {return m_running;}

	void startTimer(bool manual)
	{
		if (!m_running)
		{
			ohs.started(manual);
			m_running = true;
			QueryPerformanceCounter(&startTime);
		}
	}

	void stopTimer()
	{
		if (m_running)
		{
			// Calculate the time taken
			LARGE_INTEGER end;
			QueryPerformanceCounter(&end);
			double diff = ((double)(end.QuadPart - startTime.QuadPart)) / timerDetails.perFreq;

			m_running = false;
			ohs.stopped();

			// Add to samples
			m_samples.push_back(diff);
		}
	}

	const sampleArray& getSamples() const {return m_samples;}
	unsigned int getNumberSamples() const {return (unsigned int)m_samples.size();}

	double getTotalTime() const
	{
		double total = 0;
		sampleArray::const_iterator it;
		for(it = m_samples.begin(); it != m_samples.end(); it++)
			total += *it;

		total -= ohs.getOverhead();

#if (_MSC_VER>=1300)
		return std::max<double>(total, 0.0);
#else
		return std::_MAX(total, 0.0);
#endif
	}
	double getAverageTime() const {return (getTotalTime() / m_samples.size());}
	double getMinimumTime() const
	{
		double min = 0;
		sampleArray::const_iterator it;
		for(it = m_samples.begin(); it != m_samples.end(); it++)
		{
			if (it == m_samples.begin() || *it < min)
				min = *it;
		}
#if (_MSC_VER>=1300)
		return std::max<double>(min, 0.0);
#else
		return std::_MAX(min, 0.0);
#endif
	}
	double getMaximumTime() const
	{
		double max_val = 0;
		sampleArray::const_iterator it;
		for(it = m_samples.begin(); it != m_samples.end(); it++)
		{
			if (*it > max_val)
				max_val = *it;
		}
		return max_val;
	}
	int getTimerNo() const {return timerNo;}

protected:
	LARGE_INTEGER startTime;
	int timerNo;
	bool m_running;
	sampleArray m_samples;

	struct overheads
	{
		overheads() : starts(0), nested(0), manualStarts(0), manualNested(0) {}

		void started(bool manual)
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
		void stopped()
		{
			nested += timerDetails.curBase - base;
			manualNested += timerDetails.curManualBase - manualBase;
		}
		double getOverhead() const
		{
			return (starts + manualStarts * 1.4) * timerDetails.startstopOverhead + nested * timerDetails.nestedOverhead
				+ manualNested * timerDetails.manualNestedOverhead;
		}

		private:
			int base, nested, starts;
			int manualBase, manualNested, manualStarts;
	} ohs;

	struct TimingDetails
	{
		void Setup();

		double perFreq;
		int nextTimer;

		int curBase, curManualBase;
		double startstopOverhead, nestedOverhead, manualNestedOverhead;

#ifdef _CLOCKTICKS
		int testfun();
		double clockSpeed;
#endif
	};
	static TimingDetails timerDetails;
	// Allow friends to look at timerDetails...
	friend class CTimerFactory;
	friend struct overheads;
};

class CTimerProxy
{
public:
	CTimerProxy(CHiFreqTimer& timer) : m_timer(timer)
	{
		m_timer.startTimer(false);
	}

	virtual ~CTimerProxy()
	{
		m_timer.stopTimer();
	}
	bool operator=(CTimerProxy&) {return false;}	// remove compiler warning

protected:
	CHiFreqTimer& m_timer;
};

class CTimerFactory
{
public:
	CTimerFactory()
	{
		CHiFreqTimer::timerDetails.Setup();
	}
	virtual ~CTimerFactory()
	{
		clearTimers();
	}

	void clearTimers()
	{
		for(TimerMap::iterator it = m_map.begin(); it != m_map.end(); it++)
			delete (*it).second;

		m_map.clear();
	}

	bool isTimerPresent(const std::string& timerName)
	{
		return (m_map.find(timerName) != m_map.end());
	}

	CHiFreqTimer& getTimerChecked(const std::string& timerName)
	{
		if (!isTimerPresent(timerName))
		{
			MessageBox(0, ("Timer not found: " + timerName).c_str(), "Error", MB_OK);
			exit(0);
		}
		return getTimer(timerName);
	}

	CHiFreqTimer& getTimer(const std::string& functionName)
	{
		TimerMap::iterator pos = m_map.find(functionName);
		if (pos != m_map.end())
		{
			return *(pos->second);
		}
		else
		{
			CHiFreqTimer* newOne = new CHiFreqTimer;
			m_map.insert(TimerMap::value_type(functionName, newOne));

			return *newOne;
		}
	}

	const char* getTimerName(CHiFreqTimer& timer)
	{
		TimerMap::iterator pos = m_map.begin();
		while ((*pos).second != &timer)
			pos++;
		return ((*pos).first).c_str();
	}

	const char* timeToString(double time)
	{
		std::ostringstream ostr;
		ostr.setf(std::ios_base::fixed, std::ios_base::floatfield);
		ostr.precision(2);

		if (time < 0)
		{
			ostr << '-';
			time = -time;
		}

		if (time > 500)
			ostr << time / 1000 << " seconds ";
		else
		{
			if (time > 1)
				ostr << time << " milli secs ";
			else
				ostr << time * 1000 << " micro secs ";
		}
		tempStr = ostr.str();
		return tempStr.c_str();
	}

#ifdef _CLOCKTICKS
	int getClockSpeed() {return int(getClockSpeedActual()) + 1;}
	double getClockSpeedActual() {return CHiFreqTimer::timerDetails.clockSpeed / 1000;}

	const char* timeToCycles(double time)
	{
		std::ostringstream ostr;
		ostr.setf(std::ios_base::fixed, std::ios_base::floatfield);
		ostr.precision(2);

		double cycles = time * CHiFreqTimer::timerDetails.clockSpeed;

		if (cycles < 0)
		{
			ostr << '-';
			cycles = -cycles;
		}

		if (cycles < 1000)
			ostr << (int)cycles;
		else if (cycles < 1000000)
			ostr << cycles / 1000 << " thousand";
		else if (cycles < 1000000000)
			ostr << cycles / 1000000 << " million";
		else if (cycles < 1000000000000)
			ostr << cycles / 1000000000 << " billion";

		tempStr = ostr.str();
		return tempStr.c_str();
	}

	const char* timeToStringWithCycles(double time)
	{
		std::ostringstream ostr;
		ostr << timeToString(time);
		ostr << '(' << timeToCycles(time) << " cycles)";

		tempStr = ostr.str();
		return tempStr.c_str();
	}
#endif

	void dump(const char* file, timerSortType sort, timerFileMode mode = FILE_NEW)
	{
		std::ostream* outFStream = getOFStream(file, mode);
		_dump(*outFStream, sort);
		outputFStream(file, outFStream, mode);
	}
	void dump(timerDestType dest, timerSortType sort)
	{
		std::ostream* outStream = getOStream(dest);
		_dump(*outStream, sort);
		outputStream(outStream, dest);
	}
	void dumpCsv(const char* file, timerSortType sort, timerFileMode mode)
	{
		std::ostream* outFStream = getOFStream(file, mode);
		_dumpCsv(*outFStream, sort);
		outputFStream(file, outFStream, mode);
	}

	void dumpSamples(const char* timer, timerDestType dest)
	{
		std::ostream* outStream = getOStream(dest);
		_dumpTimer(*outStream, timer);
		outputStream(outStream, dest);
	}
	void dumpSamples(const char* timer, const char* file, timerFileMode mode = FILE_NEW)
	{
		std::ostream* outFStream = getOFStream(file, mode);
		_dumpTimer(*outFStream, timer);
		outputFStream(file, outFStream, mode);
	}
	void dumpSamplesCsv(const char* timer, const char* file, timerFileMode mode)
	{
		std::ostream* outFStream = getOFStream(file, mode);
		_dumpTimerCsv(*outFStream, timer);
		outputFStream(file, outFStream, mode);
	}
	void dumpSamples(CHiFreqTimer& timer, timerDestType dest)
		{dumpSamples(getTimerName(timer), dest);}
	void dumpSamples(CHiFreqTimer& timer, const char* file, timerFileMode mode = FILE_NEW)
		{dumpSamples(getTimerName(timer), file, mode);}
	void dumpSamplesCsv(CHiFreqTimer& timer, const char* file, timerFileMode mode)
		{dumpSamplesCsv(getTimerName(timer), file, mode);}

private:
	std::ostream* getOStream(timerDestType dest)
	{
		if ((dest == OUT_MSGBOX) || (dest == OUT_TRACE))
			return new std::ostringstream;
		else // if (dest == OUT_CONSOLE)
			return &std::cout;
	}
	void outputStream(std::ostream* outStream, timerDestType dest)
	{
		if ((dest == OUT_MSGBOX) || (dest == OUT_TRACE))
		{
			std::string str = ((std::ostringstream*)outStream)->str();

			if (dest == OUT_MSGBOX)
				MessageBox(0, str.c_str(), "Timer Output", MB_OK);
			else // if (dest == OUT_TRACE)
				OutputDebugString(str.c_str());

			delete outStream;
		}
	}

	std::ostream* getOFStream(const char* file, timerFileMode mode)
	{
		if (mode == FILE_PREPEND)
			return new std::stringstream;
		else
			return new std::ofstream(file, (mode == FILE_APPEND) ? std::ios::app : std::ios::out);
	}
	void outputFStream(const char* file, std::ostream* outStream, timerFileMode mode)
	{
		if (mode == FILE_PREPEND)
		{
			std::stringstream* strStream = (std::stringstream*)outStream;
			// Read in current file and then write out everything
			char x;
			std::ifstream istr(file);
			while (istr.get(x) && !istr.eof())
				strStream->put(x);
			istr.close();

			std::ofstream ostr(file);
			strStream->seekg(0);
			while (strStream->get(x) && !strStream->eof())
				ostr.put(x);
			ostr.close();
		}
		else
		{
			((std::ofstream*)outStream)->close();
		}
	}

	void _dump(std::ostream& ostr, timerSortType sort)
	{
		ostr << std::setw(65) << std::setfill('-') << '-' << std::endl;
		char tStr[100];
		time_t t = time(0);
		strftime(tStr, sizeof(tStr), " at %X %#x", localtime(&t));
		ostr << "Timings" << tStr << std::endl;
		ostr << std::setw(65) << std::setfill('-') << '-' << std::endl;

		if (m_map.size() == 0)
		{
			ostr << "No timers run" << std::endl;
			return;
		}
		// Sort them first...
		std::vector<sorty> sorties;
		for(TimerMap::iterator it = m_map.begin(); it != m_map.end(); it++)
			sorties.push_back(sorty(&(*it).first, (*it).second));

		if (sort == SORT_TIMES)
			std::sort(sorties.begin(), sorties.end(), timesort());
		if (sort == SORT_CALLORDER)
			std::sort(sorties.begin(), sorties.end(), ordersort());

		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, &thouSep, 1);
		// Output times
		for(std::vector<sorty>::iterator pos = sorties.begin(); pos != sorties.end(); pos++)
		{
			const std::string& functionName = *(*pos).str;
			const CHiFreqTimer* timer = (*pos).tim;
			unsigned int n = timer->getNumberSamples();

			ostr << functionName << " took " << timeToString(timer->getTotalTime())
				<< "(called ";
			numThou(ostr, n);
			ostr << " time" << ((n > 1) ? "s" : "") << ")";

			if (timer->isRunning())
				ostr << " *currently running*";

			ostr << std::endl;
		}
		ostr << std::endl;
	}

	void _dumpCsv(std::ostream& ostr, timerSortType sort)
	{
		ostr << "Timer name,Time (ms),# Calls" << std::endl;
		if (m_map.size() == 0)
		{
			ostr << "No timers run" << std::endl;
			return;
		}
		// Sort them first...
		std::vector<sorty> sorties;
		for(TimerMap::iterator it = m_map.begin(); it != m_map.end(); it++)
			sorties.push_back(sorty(&(*it).first, (*it).second));

		if (sort == SORT_TIMES)
			std::sort(sorties.begin(), sorties.end(), timesort());
		if (sort == SORT_CALLORDER)
			std::sort(sorties.begin(), sorties.end(), ordersort());

		// Output times in CSV format
		for(std::vector<sorty>::iterator pos = sorties.begin(); pos != sorties.end(); pos++)
		{
			const std::string& functionName = *(*pos).str;
			const CHiFreqTimer* timer = (*pos).tim;

			ostr << functionName << "," << timer->getTotalTime() << "," << timer->getNumberSamples();
			if (timer->isRunning())
				ostr << ",*currently running*";

			ostr << std::endl;
		}
		ostr << std::endl;
	}

	void _dumpTimer(std::ostream& ostr, const char* name)
	{
		ostr << std::setw(65) << std::setfill('-') << '-' << std::endl;
		char tStr[100];
		time_t t = time(0);
		strftime(tStr, sizeof(tStr), " at %X %#x", localtime(&t));
		ostr << name << tStr << std::endl;
		ostr << std::setw(65) << std::setfill('-') << '-' << std::endl << std::setfill(' ');

		CHiFreqTimer& timer = getTimerChecked(name);

		const std::vector<double>& samps = timer.getSamples();

		unsigned int numSamples = timer.getNumberSamples();
		int width = (int)((double)log10((double)numSamples)) + 1;
		for (unsigned int i = 0; i < numSamples; i++)
			ostr << "Sample " << std::setw(width) << i + 1 << " = " << timeToString(samps[i]) << std::endl;
		ostr << std::endl;
	}

	void _dumpTimerCsv(std::ostream& ostr, const char* name)
	{
		ostr << name << std::endl << "Sample,Time (ms)" << std::endl;

		CHiFreqTimer& timer = getTimerChecked(name);
		const std::vector<double>& samps = timer.getSamples();
		unsigned int numSamples = timer.getNumberSamples();

		for (unsigned int i = 0; i < numSamples; i++)
			ostr << i + 1 << ',' << samps[i] << std::endl;
		ostr << std::endl;
	}

	void numThou(std::ostream& ostr, int n)
	{
		if (n > 1000)
		{
			numThou(ostr, n / 1000);
			ostr << thouSep << std::setw(3) << std::setfill('0') << (n - ((n / 1000) * 1000));
		}
		else
			ostr << n;
	}

	struct sorty
	{
		sorty(const std::string* s, const CHiFreqTimer *t) {str = s; tim = t;}

		const std::string *str;
		const CHiFreqTimer *tim;
	};
	struct timesort
	{	// Highest time first
		bool operator() (const sorty & m1, const sorty & m2)
		{
			return (m1.tim->getTotalTime() > m2.tim->getTotalTime());
		}
	};
	struct ordersort
	{	// Order timers used
		bool operator() (const sorty & m1, const sorty & m2)
		{
			return (m1.tim->getTimerNo() < m2.tim->getTimerNo());
		}
	};

	typedef std::map<std::string, CHiFreqTimer*> TimerMap;
	TimerMap m_map;
	std::string tempStr;
	char thouSep;
};


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
	#define GET_TIMER(name) (*new CHiFreqTimer)
	#define T2S(time) "Timing Disabled"

	#ifdef _CLOCKTICKS
		#define CLOCK_SPEED 0
		#define T2S_WITHCYCLES(time) "Timing Disabled"
		#define T2C(time) "Timing Disabled"
	#endif

	#include <vector>

	class CHiFreqTimer
	{
	public:
		typedef std::vector<double> sampleArray;
		const sampleArray& getSamples() const {return *new sampleArray();}
		unsigned int getNumberSamples() const {return 0;}
		double getTotalTime() const {return 0;}
		double getAverageTime() const {return 0;}
		double getMinimumTime() const {return 0;}
		double getMaximumTime() const {return 0;}
	};
#endif
#endif
