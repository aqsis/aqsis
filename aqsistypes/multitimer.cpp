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
 * \brief class implementations for flexible performance timers.
 * \author Paul C. Gregory
 * \author Chris Foster
 *
 * Originally based on code published on the
 * CodeProject site:  http://www.codeproject.com/debug/multitimer.asp
*/

#include "aqsis.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "multitimer.h"

#if USE_TIMERS

namespace Aqsis {

CqTimerFactory g_timerFactory;

//------------------------------------------------------------------------------
// CqTimerFactory implementation

void CqTimerFactory::clearTimers()
{
	m_map.clear();
}

std::string CqTimerFactory::timeToString(double time)
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
	return ostr.str();
}

namespace {
struct SqSorty
{
	SqSorty(const std::string& s, const CqTimer* t)
		: str(s), tim(t)
	{}

	std::string str;
	const CqTimer* tim;
};
struct SqTimeSort
{	// Highest time first
	bool operator() (const SqSorty& m1, const SqSorty& m2)
	{
		return (m1.tim->totalTime() > m2.tim->totalTime());
	}
};
} // unnamed namespace

void CqTimerFactory::dump(std::ostream& ostr)
{
	ostr << std::setw(65) << std::setfill('-') << "-\n";
	char tStr[100];
	time_t t = time(0);
	strftime(tStr, sizeof(tStr), " at %X %#x", localtime(&t));
	ostr << "Timings" << tStr << "\n";
	ostr << std::setw(65) << std::setfill('-') << "-\n";

	if (m_map.size() == 0)
	{
		ostr << "No timers run\n";
		return;
	}
	// Sort them first...
	std::vector<SqSorty> sorties;
	for(TqTimerMap::iterator it = m_map.begin(); it != m_map.end(); it++)
		sorties.push_back(SqSorty(it->first, it->second.get()));

	std::sort(sorties.begin(), sorties.end(), SqTimeSort());

	// Output times
	for(std::vector<SqSorty>::iterator pos = sorties.begin(); pos != sorties.end(); pos++)
	{
		const std::string& timerName = pos->str;
		const CqTimer* timer = pos->tim;
		long n = timer->numSamples();

		ostr << timerName << " took " << timeToString(timer->totalTime())
		<< "(called ";
		numThou(ostr, n);
		ostr << " time" << ((n > 1) ? "s" : "") << ")";

		ostr << "\n";
	}
	ostr << "\n";
}

/// Format a number with commas as thousands separators
void CqTimerFactory::numThou(std::ostream& ostr, int n)
{
	if (n > 1000)
	{
		numThou(ostr, n / 1000);
		ostr << "," << std::setw(3) << std::setfill('0') << (n - ((n / 1000) * 1000));
	}
	else
		ostr << n;
}

} // namespace Aqsis

#endif	// USE_TIMERS

