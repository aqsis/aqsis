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


/**
 * \file memorysentry.h
 *
 * \brief Declare memory "sentry" mechanism responsible for maintaining bounded
 * global memory caches for objects such as textures.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#ifndef MEMORY_SENTRY_H_INCLUDED
#define MEMORY_SENTRY_H_INCLUDED

#include <list>
#include <stddef.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/utility.hpp>

#include "aqsistypes.h"


namespace Aqsis
{


// forward declaration
class CqMemoryMonitored;


//------------------------------------------------------------------------------
/** \brief A class to manage the total memory used by a set of objects.
 *
 * This class maintains a collection of objects, and attempts to keep the total
 * memory used by those objects below a specified limit.
 *
 * The class is noncopyable as it makes no sense for objects to be managed by
 * more than one memory sentry at the same time.
 */
class CqMemorySentry : boost::noncopyable
{
	public:
		/** \brief Type for communicating memory sizes in bytes.
		 *
		 * Should have enough precision to represent any amount of memory
		 * addressible by the machine.
		 */
		typedef ptrdiff_t TqMemorySize;

		/** \brief Create a memory sentry
		 *
		 * \param maxMemory the maximum total memory which the sentry will allow.
		 */
		CqMemorySentry(const TqMemorySize maxMemory);

		/** \brief Register an obejct to be have its memory usage overseen by
		 * this memory sentry.
		 *
		 * If the sentry runs out of memory, this object may have its
		 * zapMemory() function called to reduce the total memory.
		 *
		 * \param managedObject an object to add to the collection of managed objects.
		 */
		void registerAsManaged(const boost::shared_ptr<CqMemoryMonitored>& managedObject);

		/** \brief Add bytes to the memory sentry's total memory counter.
		 *
		 * If numBytes brings the total memory higher than the maximum limit,
		 * the sentry attempts to reclaim some memory by calling the
		 * zapMemory() functions of the CqMemoryMonitored objects which are
		 * registered with it.
		 *
		 * \param numBytes number of bytes to add to the total memory count.
		 */
		void incrementTotalMemory(const TqMemorySize numBytes);

	private:
		/// Total memory used by managed objects
		TqMemorySize m_totalMemory;
		/** Maximum amount of memory which the sentry will allow to be shared
		 * between its registered objects.
		 */
		TqMemorySize m_maxMemory;
		/** Collection of objects whose memory is watched.  These are weak
		 * pointers, as the objects in this list should be able to disappear
		 * independently of this class.
		 */
		std::list<boost::weak_ptr<CqMemoryMonitored> > m_managedList;
};


//------------------------------------------------------------------------------
/** \brief Class to be extended by all objects whose memory usage will be
 * monitored by a CqMemorySentry.
 */
class CqMemoryMonitored : boost::enable_shared_from_this<CqMemoryMonitored>
{
	public:
		/** \brief Create an object to be monitored by a CqMemorySentry
		 *
		 * \param memorySentry monitoring sentry for the memory used by the object.
		 */
		CqMemoryMonitored(const CqMemorySentry& memorySentry);

		/** \brief Ask the object to deallocate as much memory as possible.
		 *
		 * The object may choose to deallocate zero bytes if necessary.
		 *
		 * \return amount of memory which was deallocated.
		 */
		virtual CqMemorySentry::TqMemorySize zapMemory() = 0;

	protected:
		/** \brief get the memory sentry for this object.
		 *
		 * \return the memory sentry which monitors this object.
		 */
		CqMemorySentry& getMemorySentry() const;

	private:
		/// The sentry object which is monitoring the memory for this object.
		CqMemorySentry& m_memorySentry;
};


//------------------------------------------------------------------------------
// Implementation of inline functions

} // namespace Aqsis

#endif // MEMORY_SENTRY_H_INCLUDED
