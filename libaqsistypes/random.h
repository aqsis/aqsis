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
		\author Andrew Bromage (ajb@spamcop.net)
*/

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


//? Is random.h included already?
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED 1

#include	<stdlib.h>

#include	"aqsis.h"


START_NAMESPACE( Aqsis )

//----------------------------------------------------------------------
/** \class CqRandom
 * A random number generator class.
 */

class CqRandom
{
public:
    CqRandom()
    {
        Reseed( rand() );
    }
    CqRandom( unsigned int Seed )
    {
        Reseed( Seed );
    }

    /** Get a random integer in the range (0 <= value < 2^32).
     */
    unsigned int RandomInt()
    {
        if ( --m_left == 0 ) NextState();

        unsigned long y = *m_next++;
        y ^= ( y >> 11 );
        y ^= ( y << 7 ) & 0x9d2c5680UL;
        y ^= ( y << 15 ) & 0xefc60000UL;
        y ^= ( y >> 18 );
        return y;
    }

    /** Get a random integer in the specified range (0 <= value < Range).
     * \param Range Integer max value.
     */
    unsigned int RandomInt( unsigned int Range )
    {
        double n = RandomFloat( Range );
        return ( unsigned int ) ROUND(n);
    }

    /** Get a random float (0.0 <= value < 1.0).
     */
    TqFloat	RandomFloat()
    {
        return static_cast<TqFloat>( ( double ) RandomInt() * ( 1.0 / 4294967296.0 ) );
    }

    /** Get a random float in the specified range (0 <= value < Range).
     * \param Range The max value for the range.
     */
    TqFloat	RandomFloat( TqFloat Range )
    {
        return ( RandomFloat() * Range );
    }

    /** Apply a new seed value to the random number generator.
     */
    void	Reseed( unsigned int Seed )
    {
        m_state[ 0 ] = Seed & 0xffffffffUL;
        for ( int j = 1; j < N; j++ )
        {
            m_state[ j ] = ( 1812433253UL * ( m_state[ j - 1 ] ^ ( m_state[ j - 1 ] >> 30 ) ) + j );
            m_state[ j ] &= 0xffffffffUL;  /* for >32 bit machines */
        }
        m_left = 1;
        m_initf = 1;
    }

protected:
    enum {
        N = 624,
        M = 397
    };

    unsigned long m_state[ N ];
    int m_left;
    int m_initf;
    unsigned long* m_next;

#define MT_MATRIX_A	0x9908b0dfUL
#define MT_UMASK	0x80000000UL
#define MT_LMASK	0x7fffffffUL
#define MT_TWIST(u,v)	((((u) & MT_UMASK | (v) & MT_LMASK) >> 1) ^ ((v)&1UL ? MT_MATRIX_A : 0UL))

    void	NextState()
    {
        m_left = N;
        m_next = m_state;

        unsigned long* p = m_state;

        int j;
        for ( j = N - M + 1; --j; p++ )
        {
            *p = p[ M ] ^ MT_TWIST( p[ 0 ], p[ 1 ] );
        }
        for ( j = M; --j; p++ )
        {
            *p = p[ M - N ] ^ MT_TWIST( p[ 0 ], p[ 1 ] );
        }
        *p = p[ M - N ] & MT_TWIST( p[ 0 ], m_state[ 0 ] );
    }
};

//-----------------------------------------------------------------------

END_NAMESPACE( Aqsis )

#endif	// !RANDOM_H_INCLUDED
