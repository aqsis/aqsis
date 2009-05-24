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

#include	<aqsis/aqsis.h>

#include	<stdlib.h>
#include	<stdio.h>

#include	<aqsis/math/random.h>
#include	<aqsis/math/math.h>

namespace Aqsis {

// This implementation is based on the MT19937 (Mersenne Twister)
// generator by Takuji Nishimura and Makoto Matsumoto.  The original
// copyright notice follows.

/*
   A C-program for MT19937, with initialization improved 2002/2/10.
   Coded by Takuji Nishimura and Makoto Matsumoto.
   This is a faster version by taking Shawn Cokus's optimization,
   Matthe Bellew's simplification, Isaku Wada's real version.
 
   [...]
 
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
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

/// \todo <b>Code Review</b> Random number state shouldn't be stored in static storage - "independent" random number classes are not really independent in this case!  This will cause problems with reproducible sample patterns and big problems with future multithreading.
static TqUlong mt[N];   /* the array for the state vector  */
static TqInt   mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
static void init_genrand(TqUlong s)
{
	mt[0]= s & 0xffffffffUL;
	for (mti=1; mti<N; mti++)
	{
		mt[mti] =
		    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		mt[mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}

/* generates a random number on [0,0xffffffff]-interval */
static TqUlong genrand_int32(void)
{
	TqUlong  y;
	static TqUlong  mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N)
	{ /* generate N words at one time */
		TqInt kk;

		if (mti == N+1)   /* if init_genrand() has not been called, */
			init_genrand(5489UL); /* a default initial seed is used */

		for (kk=0;kk<N-M;kk++)
		{
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++)
		{
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}

	y = mt[mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}

/* generates a random number on [0,1)-real-interval */
//static TqDouble genrand_real2(void)
//{
//	return genrand_int32()*(1.0/4294967296.0);
//	/* divided by 2^32 */
//}

//----------------------------------------------------------------------
/** \class CqRandom
 * A random number generator class.
 */

CqRandom::CqRandom()
{}
CqRandom::CqRandom( TqUint Seed )
{}

/** Get a random integer in the range (0 <= value < 2^32).
 */
TqUint CqRandom::RandomInt()
{
	return genrand_int32();
}

/** Get a random integer in the specified range (0 <= value < Range).
 * \param Range Integer max value.
 */
TqUint CqRandom::RandomInt( TqUint Range )
{
	TqDouble n = RandomFloat( Range );
	return lfloor(n);
}

/** Get a random float (0.0 <= value < 1.0).
 */
TqFloat	CqRandom::RandomFloat()
{
	// Divide by 2^32 + 128.  We've got to be quite careful here, because a
	// float doesn't encompass the entire precision of the uint32 used as the
	// source of the randomness.  This means that if we just divide by 2^32
	// in double precision and truncate to a float then sometimes the float
	// will get rounded up to 1.
	//
	// Instead we've got to add the extra 128 to the denominator to ensure that
	// it always gets correctly rounded down when using the default IEEE
	// rounding mode.
	return genrand_int32()*(1.0/4294967424.0);
}

/** Get a random float in the specified range (0 <= value < Range).
 * \param Range The max value for the range.
 */
TqFloat	CqRandom::RandomFloat( TqFloat Range )
{
	return Range*RandomFloat();
}

/** Set the random internal to known value; eg. at each framebegin we might
 * want to set up the internal of random to a know value so regardless the 
 * which frame the user renders it will be able to recreate it; 
 * \param Seek The known seed number 
 */
void    CqRandom::Reseed(TqUint Seek)
{
	init_genrand((TqUlong) Seek);
}

/** Obsolete method
 */
void    CqRandom::NextState()
{}


//-----------------------------------------------------------------------

} // namespace Aqsis

