// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Implements the system specific performance timer class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<windows.h>
#include	<mmsystem.h>

#include	"aqsis.h"
#include	"perftimer.h"

START_NAMESPACE(Aqsis)


long CqPerformanceTimer::m_dTimerStart=0;
EqTimerMode CqPerformanceTimer::m_CurrMode=TimerMode_Other;


long	gaModeTimings[TimerMode_Last];

///---------------------------------------------------------------------
/// CqPerformanceTimer::~CqPerformanceTimer
/// Get the time which wasn't allocated to a mode.

CqPerformanceTimer::~CqPerformanceTimer()
{
	long tot=(timeGetTime()-m_dTimerStart);

	int i;
	for(i=TimerMode_Other; i<TimerMode_Last; i++)
		tot-=gaModeTimings[i];

	gaModeTimings[TimerMode_Other]=tot;
}


///---------------------------------------------------------------------
/// CqPerformanceTimer::Init
/// Initialise the timer.

void CqPerformanceTimer::Init()
{
	m_dTimerStart = timeGetTime();

	int i;
	for(i=0; i<TimerMode_Last; i++)	gaModeTimings[i]=0;
}


///---------------------------------------------------------------------
/// CqPerformanceTimerMode::CqPerformanceTimerMode
/// Constructor.

CqPerformanceTimerMode::CqPerformanceTimerMode(EqTimerMode Mode)	
{
	m_OldMode=m_CurrMode;
	m_CurrMode=Mode;
	timeBeginPeriod(1);
	m_dBaseTime=timeGetTime();
}


///---------------------------------------------------------------------
/// CqPerformanceTimerMode::~CqPerformanceTimerMode
/// Destructor.

CqPerformanceTimerMode::~CqPerformanceTimerMode()	
{
	gaModeTimings[m_CurrMode]+=(timeGetTime()-m_dBaseTime);
	timeEndPeriod(1);
	m_CurrMode=m_OldMode;
}

//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
