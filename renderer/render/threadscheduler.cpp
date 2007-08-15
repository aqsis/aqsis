/* Aqsis
 *
 * Copyright (C) 2007 Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include	"threadscheduler.h"


START_NAMESPACE( Aqsis );

/**
 * \brief Wrapper class for a unit to be processed by a thread.
 *
 * It adds a notifying call to the scheduler to know when the thread
 * finished.
 */
class UnitWrapper
{
public:
	UnitWrapper(CqThreadScheduler* threadScheduler, const boost::function0<void>& unit) :
		m_threadScheduler(threadScheduler), m_unit(unit)
	{
	}

	void operator()()
	{
//		fprintf(stderr, "! executing thread\n");
		m_unit();
//		fprintf(stderr, "! finishing thread\n");
		m_threadScheduler->notifyWorkUnitFinished();
	}

private:
	CqThreadScheduler* m_threadScheduler;
	const boost::function0<void>& m_unit;
};



CqThreadScheduler::CqThreadScheduler(TqInt maxThreads) :
	m_maxThreads(maxThreads), m_activeThreads(0)
{
//	fprintf(stderr, "* Thread scheduler started with max threads=%d\n", m_maxThreads);
}


CqThreadScheduler::~CqThreadScheduler()
{
}


void CqThreadScheduler::addWorkUnit(const boost::function0<void>& unit)
{
//	fprintf(stderr, "> Adding work unit, will block?\n");

/*
	boost::mutex::scoped_lock lock(m_mutexCondition);
	if (m_activeThreads >= m_maxThreads)
	{
		fprintf(stderr, " - blocked: max threads=%d, active threads=%d'\n", m_maxThreads, m_activeThreads);
		m_threadsAvailable.wait(lock);
	}
	fprintf(stderr, " - passed wait condition 'threads available'\n");
*/

	{ 
//		fprintf(stderr, " - lock in ++activeThreads\n");

		boost::mutex::scoped_lock lock2(m_mutexActiveThreads);
		++m_activeThreads;
//		fprintf(stderr, " - thread started, current active threads: %d\n", m_activeThreads);
	}

	m_threadGroup.create_thread(UnitWrapper(this, unit));
}


void CqThreadScheduler::notifyWorkUnitFinished()
{
// 	fprintf(stderr, "< Thread finished\n");
	m_threadsAvailable.notify_one();
	{
//		fprintf(stderr, " - lock in --activeThreads\n");

		boost::mutex::scoped_lock lock(m_mutexActiveThreads);
		--m_activeThreads;
//		fprintf(stderr, " - current active threads: %d\n", m_activeThreads);
	}
}


void CqThreadScheduler::joinAll()
{
//	fprintf(stderr, "* Join all\n");
	m_threadGroup.join_all();
}

END_NAMESPACE( Aqsis );
