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

#include "aqsis.h"

#include "multitimer.h"
#include "multitimer_system.h"

#if USE_TIMERS

namespace Aqsis {

CqTimerFactory timerFactory;
CqHiFreqTimerBase::SqTimingDetails CqHiFreqTimerBase::timerDetails;

const CqHiFreqTimerBase::sampleArray& CqHiFreqTimerBase::getSamples() const
{
	return m_samples;
}


unsigned int CqHiFreqTimerBase::getNumberSamples() const
{
	return (unsigned int)m_samples.size();
}


double CqHiFreqTimerBase::getTotalTime() const
{
	double total = 0;
	sampleArray::const_iterator it;
	for(it = m_samples.begin(); it != m_samples.end(); it++)
		total += *it;

	total -= ohs.getOverhead();

	return std::max<double>(total, 0.0);
}

double CqHiFreqTimerBase::getAverageTime() const
{
	return (getTotalTime() / m_samples.size());
}


double CqHiFreqTimerBase::getMinimumTime() const
{
	double min = 0;
	sampleArray::const_iterator it;
	for(it = m_samples.begin(); it != m_samples.end(); it++)
	{
		if (it == m_samples.begin() || *it < min)
			min = *it;
	}
	return std::max<double>(min, 0.0);
}

double CqHiFreqTimerBase::getMaximumTime() const
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

int CqHiFreqTimerBase::getTimerNo() const
{
	return timerNo;
}


CqTimerFactory::CqTimerFactory()
{
	CqHiFreqTimerBase::timerDetails.Setup();
}

CqTimerFactory::~CqTimerFactory()
{
	clearTimers();
}


void CqTimerFactory::clearTimers()
{
	m_map.clear();
}


const char* CqTimerFactory::getTimerName(const boost::shared_ptr<CqHiFreqTimerBase>& timer)
{
	TqTimerMap::iterator pos = m_map.begin();
	while ((*pos).second != timer)
		pos++;
	return ((*pos).first).c_str();
}

const char* CqTimerFactory::timeToString(double time)
{
	std::ostringstream ostr;
	ostr.setf(std::ios_base::fixed, std::ios_base::floatfield)
		;
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

void CqTimerFactory::dump(const char* file, EqTimerSortType sort, EqTimerFileMode mode)
{
	std::ostream* outFStream = getOFStream(file, mode);
	_dump(*outFStream, sort);
	outputFStream(file, outFStream, mode);
}
void CqTimerFactory::dump(EqTimerDestType dest, EqTimerSortType sort)
{
	std::ostream* outStream = getOStream(dest);
	_dump(*outStream, sort);
	outputStream(outStream, dest);
}
void CqTimerFactory::dumpCsv(const char* file, EqTimerSortType sort, EqTimerFileMode mode)
{
	std::ostream* outFStream = getOFStream(file, mode);
	_dumpCsv(*outFStream, sort);
	outputFStream(file, outFStream, mode);
}

void CqTimerFactory::dumpSamples(const char* timer, EqTimerDestType dest)
{
	std::ostream* outStream = getOStream(dest);
	_dumpTimer(*outStream, timer);
	outputStream(outStream, dest);
}
void CqTimerFactory::dumpSamples(const char* timer, const char* file, EqTimerFileMode mode)
{
	std::ostream* outFStream = getOFStream(file, mode);
	_dumpTimer(*outFStream, timer);
	outputFStream(file, outFStream, mode);
}
void CqTimerFactory::dumpSamplesCsv(const char* timer, const char* file, EqTimerFileMode mode)
{
	std::ostream* outFStream = getOFStream(file, mode);
	_dumpTimerCsv(*outFStream, timer);
	outputFStream(file, outFStream, mode);
}
void CqTimerFactory::dumpSamples(const boost::shared_ptr<CqHiFreqTimerBase>& timer, EqTimerDestType dest)
{
	dumpSamples(getTimerName(timer), dest);
}
void CqTimerFactory::dumpSamples(const boost::shared_ptr<CqHiFreqTimerBase>& timer, const char* file, EqTimerFileMode mode)
{
	dumpSamples(getTimerName(timer), file, mode);
}
void CqTimerFactory::dumpSamplesCsv(const boost::shared_ptr<CqHiFreqTimerBase>& timer, const char* file, EqTimerFileMode mode)
{
	dumpSamplesCsv(getTimerName(timer), file, mode);
}

std::ostream* CqTimerFactory::getOStream(EqTimerDestType dest)
{
	if ((dest == OUT_MSGBOX) || (dest == OUT_TRACE))
		return new std::ostringstream;
	else // if (dest == OUT_CONSOLE)
		return &std::cout;
}

void CqTimerFactory::outputStream(std::ostream* outStream, EqTimerDestType dest)
{
/*	if ((dest == OUT_MSGBOX) || (dest == OUT_TRACE))
	{
		std::string str = ((std::ostringstream*)outStream)->str();

		if (dest == OUT_MSGBOX)
			MessageBox(0, str.c_str(), "Timer Output", MB_OK);
		else // if (dest == OUT_TRACE)
			OutputDebugString(str.c_str());
		delete outStream;
	}*/
}

std::ostream* CqTimerFactory::getOFStream(const char* file, EqTimerFileMode mode)
{
	if (mode == FILE_PREPEND)
		return new std::stringstream;
	else
		return new std::ofstream(file, (mode == FILE_APPEND) ? std::ios::app : std::ios::out);
}
void CqTimerFactory::outputFStream(const char* file, std::ostream* outStream, EqTimerFileMode mode)
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
		while (strStream->get
				(x) && !strStream->eof())
			ostr.put(x);
		ostr.close();
	}
	else
	{
		((std::ofstream*)outStream)->close();
	}
}

void CqTimerFactory::_dump(std::ostream& ostr, EqTimerSortType sort)
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
	std::vector<SqSorty> sorties;
	for(TqTimerMap::iterator it = m_map.begin(); it != m_map.end(); it++)
		sorties.push_back(SqSorty((*it).first, (*it).second));

	if (sort == SORT_TIMES)
		std::sort(sorties.begin(), sorties.end(), SqTimeSort());
	if (sort == SORT_CALLORDER)
		std::sort(sorties.begin(), sorties.end(), SqOrderSort());

	//GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, &thouSep, 1);
	thouSep = ',';
	// Output times
	for(std::vector<SqSorty>::iterator pos = sorties.begin(); pos != sorties.end(); pos++)
	{
		const std::string& functionName = (*pos).str;
		const boost::shared_ptr<CqHiFreqTimerBase>& timer = (*pos).tim;
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

void CqTimerFactory::_dumpCsv(std::ostream& ostr, EqTimerSortType sort)
{
	ostr << "Timer name,Time (ms),# Calls" << std::endl;
	if (m_map.size() == 0)
	{
		ostr << "No timers run" << std::endl;
		return;
	}
	// Sort them first...
	std::vector<SqSorty> sorties;
	for(TqTimerMap::iterator it = m_map.begin(); it != m_map.end(); it++)
		sorties.push_back(SqSorty((*it).first, (*it).second));

	if (sort == SORT_TIMES)
		std::sort(sorties.begin(), sorties.end(), SqTimeSort());
	if (sort == SORT_CALLORDER)
		std::sort(sorties.begin(), sorties.end(), SqOrderSort());

	// Output times in CSV format
	for(std::vector<SqSorty>::iterator pos = sorties.begin(); pos != sorties.end(); pos++)
	{
		const std::string& functionName = (*pos).str;
		const boost::shared_ptr<CqHiFreqTimerBase>& timer = (*pos).tim;

		ostr << functionName << "," << timer->getTotalTime() << "," << timer->getNumberSamples();
		if (timer->isRunning())
			ostr << ",*currently running*";

		ostr << std::endl;
	}
	ostr << std::endl;
}

void CqTimerFactory::_dumpTimer(std::ostream& ostr, const char* name)
{
	ostr << std::setw(65) << std::setfill('-') << '-' << std::endl;
	char tStr[100];
	time_t t = time(0);
	strftime(tStr, sizeof(tStr), " at %X %#x", localtime(&t));
	ostr << name << tStr << std::endl;
	ostr << std::setw(65) << std::setfill('-') << '-' << std::endl << std::setfill(' ');

	boost::shared_ptr<CqHiFreqTimerBase> timer = getTimerChecked(name);

	const std::vector<double>& samps = timer->getSamples();

	unsigned int numSamples = timer->getNumberSamples();
	int width = (int)((double)log10((double)numSamples)) + 1;
	for (unsigned int i = 0; i < numSamples; i++)
		ostr << "Sample " << std::setw(width) << i + 1 << " = " << timeToString(samps[i]) << std::endl;
	ostr << std::endl;
}

void CqTimerFactory::_dumpTimerCsv(std::ostream& ostr, const char* name)
{
	ostr << name << std::endl << "Sample,Time (ms)" << std::endl;

	boost::shared_ptr<CqHiFreqTimerBase> timer = getTimerChecked(name);
	const std::vector<double>& samps = timer->getSamples();
	unsigned int numSamples = timer->getNumberSamples();

	for (unsigned int i = 0; i < numSamples; i++)
		ostr << i + 1 << ',' << samps[i] << std::endl;
	ostr << std::endl;
}

void CqTimerFactory::numThou(std::ostream& ostr, int n)
{
	if (n > 1000)
	{
		numThou(ostr, n / 1000);
		ostr << thouSep << std::setw(3) << std::setfill('0') << (n - ((n / 1000) * 1000));
	}
	else
		ostr << n;
}

boost::shared_ptr<CqHiFreqTimerBase> CqHiFreqTimerBase::createTimer()
{
	return(boost::shared_ptr<CqHiFreqTimerBase>(new CqHiFreqTimer));
}

} // namespace Aqsis

#endif	// USE_TIMERS

