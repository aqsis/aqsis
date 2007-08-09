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
		\brief Declares system specific classes required for the flexible performance timers.
		\author	Paul C. Gregory - originally based on code published on the CodeProject site
				http://www.codeproject.com/debug/multitimer.asp
*/

//? Is color.h included already?
#ifndef	SYSTEM_MULTITIMER_H_INCLUDED
#define SYSTEM_MULTITIMER_H_INCLUDED

#ifdef USE_TIMERS

#include "aqsis.h"

#include <windows.h>

#include "multitimer.h"

namespace Aqsis {

class COMMON_SHARE CqHiFreqTimer : public CqHiFreqTimerBase
{
	public:
		inline CqHiFreqTimer();
		inline virtual ~CqHiFreqTimer();

		inline void startTimer(bool manual);
		inline void stopTimer();

	protected:
		LARGE_INTEGER startTime;
};

CqHiFreqTimer::CqHiFreqTimer()
{
	startTime.QuadPart = 0;
}

CqHiFreqTimer::~CqHiFreqTimer()
{
	if (m_running)
		stopTimer();
}

void CqHiFreqTimer::startTimer(bool manual)
{
	if (!m_running)
	{
		ohs.started(manual);
		m_running = true;
		QueryPerformanceCounter(&startTime);
	}
}

void CqHiFreqTimer::stopTimer()
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

} // namespace Aqsis

#endif

#endif // SYSTEM_MULTITIMER_H_INCLUDED
