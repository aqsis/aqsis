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
		\brief Declares the system specific performance timer class.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is attributes.h included already?
#ifndef PERFTIME_H_INCLUDED
//{
#define PERFTIME_H_INCLUDED 1

#include	"specific.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

enum EqTimerMode
{
	TimerMode_Other=0,
	TimerMode_Shade,
	TimerMode_Split,

	TimerMode_Last
};

 
extern long	gaModeTimings[TimerMode_Last];

class CqPerformanceTimer
{
	public:
			CqPerformanceTimer()	{}
			~CqPerformanceTimer();

	void	Init();

	protected:
	static EqTimerMode	m_CurrMode;

	private:
	static long	m_dTimerStart;
};


class CqPerformanceTimerMode : public CqPerformanceTimer
{
	public:
			CqPerformanceTimerMode(EqTimerMode Mode);
			~CqPerformanceTimerMode();

	private:

		EqTimerMode	m_OldMode;
		long		m_dBaseTime;
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

//}  // End of #ifdef ATTRIBUTES_H_INCLUDED
#endif

