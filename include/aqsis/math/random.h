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
		\brief Declares the CqRandom class responsible for producing random numbers.
		\author Andrew Bromage (ajb@spamcop.net)
*/

// This implementation is based on the MT19937 (Mersenne Twister)
// generator by Takuji Nishimura and Makoto Matsumoto.  The original
// copyright notice follows.

//? Is random.h included already?
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED 1

#include	<stdlib.h>

#include	<aqsis/aqsis.h>


namespace Aqsis {
/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.
 
   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).
 
   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          
 
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
 
     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
 
     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
 
     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.
 
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 
   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

//----------------------------------------------------------------------
/** \class CqRandom
 * A random number generator class.
 */

class AQSIS_MATH_SHARE CqRandom
{
	public:
		CqRandom();

		CqRandom( TqUint Seed );

		/** Get a random integer in the range (0 <= value < 2^32).
		 */
		TqUint RandomInt();


		/** Get a random integer in the specified range (0 <= value < Range).
		 * \param Range Integer max value.
		 */
		TqUint RandomInt( TqUint Range );

		/** Get a random float (0.0 <= value < 1.0).
		 */
		TqFloat	RandomFloat();


		/** Get a random float in the specified range (0 <= value < Range).
		 * \param Range The max value for the range.
		 */
		TqFloat	RandomFloat( TqFloat Range );

		void    Reseed(TqUint Seek);
	protected:
		void NextState();
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !RANDOM_H_INCLUDED
