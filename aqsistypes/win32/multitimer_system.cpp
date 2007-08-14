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
#ifdef USE_TIMERS

#include "aqsis.h"


#include "multitimer_system.h"
#include "exception.h"

namespace Aqsis {

void CqHiFreqTimer::SqTimingDetails::Setup()
{
	LARGE_INTEGER freq;
	if (QueryPerformanceFrequency(&freq) == 0)
	{
		throw(XqInternal("Failure to query performance timer", "", __FILE__, __LINE__));
	}
	perFreq = ((double)freq.QuadPart) / 1000;
	nextTimer = curBase = curManualBase = 0;

	int i, loops = 1000;
	void* Fred = GetCurrentThread();
	int CurPri = GetThreadPriority(Fred);
	SetThreadPriority(Fred, THREAD_PRIORITY_HIGHEST);
	Sleep(100);

	LARGE_INTEGER pTest;
	{
		TIME_SCOPE("ss overhead")
		for (i = 0; i < loops; i++)
		{
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
			QueryPerformanceCounter(&pTest);
		}
	}
	startstopOverhead = GET_TIMER("ss overhead")->getTotalTime() / (loops * 10);
	CLEAR_TIMERS

	for (i = 0; i < loops; i++)
	{
		TIME_SCOPE("nested")
		{
			TIME_SCOPE("sub")
		}
	}
	nestedOverhead = GET_TIMER("nested")->getAverageTime();
	CLEAR_TIMERS
	for (i = 0; i < loops; i++)
	{
		TIME_SCOPE("nested")
		{
			TIMER_START("sub")
			TIMER_STOP("sub")
		}
	}
	manualNestedOverhead = GET_TIMER("nested")->getAverageTime();// * 1.4;
	CLEAR_TIMERS

	SetThreadPriority(Fred, CurPri);
}

} // namespace Aqsis

#endif