/* Aqsis - threadscheduler.cpp
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


namespace Aqsis {

/**
 * \brief Wrapper class for a unit to be processed by a thread.
 *
 * It adds a notifying call to the scheduler to know when the thread
 * finished.
 */
class CqUnitWrapper
{
public:
	CqUnitWrapper(CqThreadScheduler* threadScheduler, const boost::function0<void>& unit) :
		m_threadScheduler(threadScheduler), m_unit(unit)
	{
	}

	void operator()()
	{
		m_unit();
		m_threadScheduler->notifyWorkUnitFinished();
	}

private:
	CqThreadScheduler* m_threadScheduler;
	const boost::function0<void> m_unit;
};



CqThreadScheduler::CqThreadScheduler(TqInt maxThreads) :
	m_maxThreads(maxThreads), m_activeThreads(0)
{
}


CqThreadScheduler::~CqThreadScheduler()
{
}


void CqThreadScheduler::addWorkUnit(const boost::function0<void>& unit)
{
#ifdef	ENABLE_THREADING
	boost::mutex::scoped_lock lock(m_mutexCondition);
	if (m_activeThreads >= m_maxThreads)
	{
		m_threadsAvailable.wait(lock);
	}

	{ 
		boost::mutex::scoped_lock lock(m_mutexActiveThreads);
		++m_activeThreads;
	}

	m_threadGroup.create_thread( CqUnitWrapper(this, unit) );
#else // ENABLE_THREADING
	// If not threading, just run the process asynchronously.
	unit();
#endif
}


void CqThreadScheduler::notifyWorkUnitFinished()
{
#ifdef	ENABLE_THREADING
	{
		boost::mutex::scoped_lock lock(m_mutexActiveThreads);
		--m_activeThreads;
	}
	m_threadsAvailable.notify_one();
#endif
}


void CqThreadScheduler::joinAll()
{
#ifdef	ENABLE_THREADING
	m_threadGroup.join_all();
#endif
}


} // namespace Aqsis
