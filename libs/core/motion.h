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
		\brief Declares the CqMotionSpec template class for handling any class capable of being motion blurred.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef MOTION_H_INCLUDED
#define MOTION_H_INCLUDED 1

#include	<vector>

#include	<aqsis/aqsis.h>

#include	<aqsis/ri/ri.h>

namespace Aqsis {


//----------------------------------------------------------------------
/** \class CqMotionSpec
 * Abstract base class from which all classes supporting motion blur should derive.
 */

template <class T>
class CqMotionSpec
{
	public:
		/** Default constructor.
		 * \param a The default value for the motion object, used when expanding the timeslots.
		 */
		CqMotionSpec( const T& a )
		{
			m_DefObject = a;
			//AddTimeSlot(0.0f, a);
		}
		/** Copy constructor.
		 */
		CqMotionSpec( const CqMotionSpec<T>& From )
		{
			*this = From;
		}
		virtual	~CqMotionSpec()
		{
			//for(std::vector<T>::iterator i=m_aObjects.begin(); i<m_aObjects.end(); i++)
			//	ClearMotionObject((*i));
		}

		/** Assignment operator.
		 */
		void	operator=( const CqMotionSpec<T>& From )
		{
			m_aTimes.clear();
			m_aObjects.clear();
			for ( std::vector<TqFloat>::const_iterator t = From.m_aTimes.begin(); t<From.m_aTimes.end(); t++ )
				m_aTimes.push_back( *t );
			for( typename std::vector<T>::const_iterator o = From.m_aObjects.begin(); o<From.m_aObjects.end(); o++ )
				m_aObjects.push_back( *o );
			m_DefObject = From.m_DefObject;
		}

		/** Put the specified motion object at the specified time, create a new timeslot if none exists,
		 * else overwrite the existing value.
		 * \param time The frame time to place the new object at.
		 * \param Object The new object to put in the timeslot.
		 */
		void	AddTimeSlot( TqFloat time, const T& Object )
		{
			TqInt iIndex;
			if ( cTimes() == 0 )
			{
				m_aTimes.push_back( time );
				m_aObjects.push_back( Object );
				return ;
			}

			if ( TimeSlotExists( time, iIndex ) )
			{
				ClearMotionObject( m_aObjects[ iIndex ] );
				m_aObjects[ iIndex ] = Object;
			}
			else
			{
				// Insert the timeslot at the proper place.
				std::vector<TqFloat>::iterator itime = m_aTimes.begin();
				typename std::vector<T>::iterator iobj = m_aObjects.begin();
				while ( itime != m_aTimes.end() && *itime < time )
					itime++, iobj++;
				m_aTimes.insert( itime, time );
				m_aObjects.insert( iobj, Object );
			}
		}
		/** Combine the specified motion object with any existing one at the specified time.
		 * Will create a new timeslot if none exists.
		 * \param time The time of the relevant timeslot.
		 * \param Object The new obejct to combine with any existing one.
		 */
		void	ConcatTimeSlot( TqFloat time, const T& Object )
		{
			TqInt iIndex;
			if ( TimeSlotExists( time, iIndex ) )
				m_aObjects[ iIndex ] = ConcatMotionObjects( m_aObjects[ iIndex ], Object );
			else
			{
				// Add a new slot a nd set it to the default value before concatenating the
				// specified value.
				AddTimeSlot( time, m_DefObject );
				TimeSlotExists( time, iIndex );
				m_aObjects[ iIndex ] = ConcatMotionObjects( m_aObjects[ iIndex ], Object );
			}
		}
		/** Combine the specified motion object with all timeslots.
		 * \param Object The object to combine.
		 */
		void	ConcatAllTimeSlots( const T& Object )
		{
			for ( typename std::vector<T>::iterator i = m_aObjects.begin(); i<m_aObjects.end(); i++ )
				( *i ) = ConcatMotionObjects( ( *i ), Object );
		}
		/** Get the frame time at the specified timeslot index.
		 * \param index The timeslot index/
		 * \return Float frame time.
		 */
		TqFloat	Time( TqInt index )
		const
		{
			if ( m_aTimes.size() == 0 )
				return ( 0.0f );
			else if ( index < 0 )
				return ( m_aTimes[ 0 ] );
			else if ( index < cTimes() )
				return ( m_aTimes[ index ] );
			else
				return ( m_aTimes.back() );
		}
		/** Get the number of timeslots.
		 * \return Integer timeslot count.
		 */
		TqInt	cTimes() const
		{
			return ( m_aTimes.size() );
		}
		/** Return whether this motion block is actually motion blurred or not.
		 * \return Boolean indicating motion blurred.
		 */
		bool	fMotionBlurred() const
		{
			return ( cTimes() > 1 );
		}
		/** Get the index of the timeslot for the specified time, and if not exact, the fractional
		 * distance between it and the following timeslot 0-1.
		 * \param time Float frame time.
		 * \param iIndex Reference to the integer index to fill in.
		 * \param Fraction Reference to the float fraction to fill in.
		 * \return Boolean indicating the index is exact.
		 */
		bool	GetTimeSlot( TqFloat time, TqInt& iIndex, TqFloat& Fraction ) const
		{
			assert( cTimes() > 0 );

			if ( time >= m_aTimes.back() )
			{
				iIndex = m_aTimes.size() - 1;
				return ( true );
			}
			else if ( time <= m_aTimes.front() )
			{
				iIndex = 0;
				return ( true );
			}
			else
			{
				// Find the appropriate time span.
				iIndex = 0;
				while ( time >= m_aTimes[ iIndex + 1 ] )
					iIndex += 1;
				Fraction = ( time - m_aTimes[ iIndex ] ) / ( m_aTimes[ iIndex + 1 ] - m_aTimes[ iIndex ] );
				return ( m_aTimes[ iIndex ] == time );
			}
		}
		/** Get the index of the timeslot for the specified time, if it exists.
		 * \param time Float frame time.
		 * \param iIndex Reference to the integer index to fill in.
		 * \return Boolean indicating the index exists.
		 */
		bool	TimeSlotExists( TqFloat time, TqInt& iIndex ) const
		{
			//assert( cTimes() > 0 );
			//assert(time>=0.0f);

			// Find the appropriate time span.
			iIndex = 0;
			while ( iIndex < cTimes() && time != m_aTimes[ iIndex ] )
				iIndex += 1;
			return ( iIndex < cTimes() );
		}
		/** Get the motion object at the specified time using linearinterpolation as appropriate.
		 * \param time Float frame time.
		 * \return New object representing the motion object at the specified time.
		 */
		T	GetMotionObjectInterpolated( TqFloat time ) const
		{
			TqInt iIndex;
			TqFloat Fraction;
			if ( GetTimeSlot( time, iIndex, Fraction ) )
				return ( m_aObjects[ iIndex ] );
			else
				return ( LinearInterpolateMotionObjects( Fraction, m_aObjects[ iIndex ], m_aObjects[ iIndex + 1 ] ) );
		}

		/** Get the motion object at the specified time, will only work if the timeslot exists.
		 * \param time Float frame time.
		 * \return Reference to the object at the appropriate timeslot.
		 */
		const	T&	GetMotionObject( TqFloat time ) const
		{
			TqInt iIndex;
			TqFloat Fraction;
			if ( GetTimeSlot( time, iIndex, Fraction ) )
				return ( m_aObjects[ iIndex ] );
			else
			{
				assert( false );
				return ( m_DefObject );
			}
		}

		const T& GetDefaultObject() const
		{
			return(m_DefObject);
		}

		/** Get the motion object at the specified index.
		 * \param iIndex Index in the array of time slots.
		 * \return Reference to the object at the appropriate timeslot.
		 */
		const	T&	GetMotionObject( TqInt iIndex ) const
		{
			assert( iIndex >= 0 && iIndex < cTimes() );
			return ( m_aObjects[ iIndex ] );
		}

		/** Set the motion object to use as the default when expanding the timeslot list.
		 * \param a The new default motion object.
		 */
		void	SetDefaultObject( const T& a )
		{
			m_DefObject = a;
		}

		/** Empty all keyframes
		 *
		 */
		void	Reset()
		{
			m_aTimes.clear();
			typename std::vector<T>::iterator i;
			for(i=m_aObjects.begin(); i!=m_aObjects.end(); i++)
				ClearMotionObject(*i);
			m_aObjects.clear();
		}

		/** Pure virtual, overridden by deriving classes. Clear a motion object.
		 */
		virtual	void	ClearMotionObject( T& A ) const = 0;
		/** Pure virtual, overridden by deriving classes. Combine motion obects.
		 */
		virtual	T	ConcatMotionObjects( const T& A, const T& B ) const = 0;
		/** Pure virtual, overridden by deriving classes. Linear interpolate between motion objects.
		 */
		virtual	T	LinearInterpolateMotionObjects( TqFloat Fraction, const T& A, const T& B ) const = 0;

	private:
		std::vector<TqFloat>	m_aTimes;		///< Array of float timeslot times.
		std::vector<T>	m_aObjects;		///< Array of motion objects for each timeslot.
		T	m_DefObject;	///< Default motion object, used when expanding the list of timeslots.
}
;


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif // MOTION_H_INCLUDED
