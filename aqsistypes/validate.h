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
		\brief Global Aqsis include to include all files required within Aqsis
		\author Matthaeus G. Chajdas (Matthaeus@darkside-conflict.net)
*/

//? Is .h included already?
#ifndef AQERROR_H_INCLUDED
#define AQERROR_H_INCLUDED 1

//! A callback/functor class to perform range checks
/**
 * This class is an abstract base class. In order to use the range check
 * class, you need to derive a class from this (see CqLogRangeCheckCallback in ri.cpp)
 * The function call takes an argument which is the result of a CheckMinMax function call.
*/

class CqRangeCheckCallback
{
	public:
		virtual ~CqRangeCheckCallback()
		{
		};
		virtual void operator()( int res) = 0;
		virtual void Call( int res )
		{
			this->operator()( res );
		}

		enum {	LOWER_BOUND_HIT,
		       UPPER_BOUND_HIT,
		       VALID	}	EqRangeCheck;
};

//! Range check funtion using CqRangeCheckCallback
/**
 *	Do the range check
 *
 *	Calls a CqRangeCheckCallback functor class, which is responsible for the output.
 *  \return A boolean indicating whether the variable val is inside the range [min,max]
 */
template<class T>
bool CheckMinMax( const T& val, const T& min, const T& max, CqRangeCheckCallback* callBack)
{
	if( val < min )
	{
		(*callBack)( CqRangeCheckCallback::LOWER_BOUND_HIT );
		return false;
	}

	if( val > max )
	{
		(*callBack)( CqRangeCheckCallback::UPPER_BOUND_HIT );
		return false;
	}

	(*callBack)( CqRangeCheckCallback::VALID );
	return true;
}

#endif


