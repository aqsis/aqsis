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
		\brief Declares the CqRandom class responsible for producing random numbers.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is random.h included already?
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED 1

#include	<stdlib.h>

#include	"specific.h"

#include	"ri.h"

#define		_qShareName	CORE
#include	"share.h"

START_NAMESPACE(Aqsis)

//----------------------------------------------------------------------
/** \class CqRandom
 * A random number generator class.
 */

class _qShareC CqRandom
{
	public:
		_qShareM				CqRandom()					{Reseed(rand());}
		_qShareM				CqRandom(unsigned int Seed)	{Reseed(Seed);}

								/** Get a random integer in the range (0 < value < 2^32).
								 */
		_qShareM	unsigned int RandomInt()				{return(m_NextValue=m_NextValue*1664525L+1013904223L);}
								/** Get a random integer in the specified range (0 < value < Range).
								 * \param Range Integer max value.
								 */
		_qShareM	unsigned int RandomInt(unsigned int Range)
								{
									__int64 n=static_cast<unsigned __int64>(RandomInt())*static_cast<__int64>(Range);
									return(static_cast<unsigned int>(n>>32));
								}

								/** Get a random float (0.0 < value < 1.0).
								 * \param Range Integer max value.
								 */
		_qShareM	TqFloat		RandomFloat()
								{
									union {	unsigned long Int; float Float; } IEEEFloatMaker;
									IEEEFloatMaker.Int=(RandomInt()&0x7FFFFF)|0x3F800000;	// Or in 23 bits of mantissa, now 1.0 .. 1.999+
									return(IEEEFloatMaker.Float-1.0f);						// Return 0.0 .. 0.999+
								}
								/** Get a random float in the specified range (0 < value < Range).
								 * \param Range The max value for the range.
								 */
		_qShareM	TqFloat		RandomFloat(float Range)	{return(RandomFloat()*Range);}

								/** Apply a new seed value to the random number generator.
								 */
		_qShareM	void		Reseed(unsigned int Seed)	{m_NextValue=Seed;}

	protected:
					unsigned int	m_NextValue;
};

//-----------------------------------------------------------------------

END_NAMESPACE(Aqsis)

#endif	// !RANDOM_H_INCLUDED
