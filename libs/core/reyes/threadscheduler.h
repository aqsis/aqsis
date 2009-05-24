/* Aqsis - threadscheduler.h
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

/** \file
 *
 * \brief File in which the thread scheduler is implemented.
 *
 * \author Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 */

#ifndef THREADSCHEDULER_H_INCLUDED
#define THREADSCHEDULER_H_INCLUDED 1

#include	<aqsis/aqsis.h>
#include	<boost/function.hpp>

#ifdef	ENABLE_THREADING
#include	<boost/thread/thread.hpp>
#include	<boost/thread/mutex.hpp>
#include	<boost/thread/condition.hpp>
#endif

namespace Aqsis { 


/**
 * \brief Class to schedule threads processing work units
 */
class CqThreadScheduler
{
public:
	/** Default constructor */
	CqThreadScheduler(TqInt maxThreads);
	/** Destructor */
	~CqThreadScheduler();

	/** Add a work unit to be processed */
	void addWorkUnit(const boost::function0<void>& unit);
	/** Notify that a work unit has been processed */
	void notifyWorkUnitFinished();
	/** Join all the threads, this is, wait for all the threads to
	 * finish their job before continuing */
	void joinAll();

private:
	/// Maximum number of threads to run
	TqInt m_maxThreads;
	/// Current number of active threads running
	TqInt m_activeThreads;
#ifdef	ENABLE_THREADING
	/// Hold the group of active threads
	boost::thread_group m_threadGroup;
	/// Mutex for condition
	boost::mutex m_mutexCondition;
	/// Mutex for active threads
	boost::mutex m_mutexActiveThreads;
	/// Condition to wait for available threads
	boost::condition m_threadsAvailable;
#endif
};


} // namespace Aqsis

#endif
