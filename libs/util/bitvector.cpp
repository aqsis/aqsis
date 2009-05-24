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
		\brief Implements the CqBitVector class for handling efficient bit vectors of any length.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<iomanip>
#include	<aqsis/aqsis.h>

#include <aqsis/util/bitvector.h>

namespace Aqsis {

//----------------------------------------------------------------------
/** Return a new bitvector which is the intersection of the current one and the one specified.
 * If they are of different sizes the result will be equal to the smallest of the two.
 * \param from CqBitVector reference to perform the operation with.
 */

CqBitVector&	CqBitVector::Intersect( CqBitVector& from )
{
	TqInt size = ( from.m_cLength < m_cLength ) ? from.m_cLength : m_cLength;
	SetSize( size );
	TqInt numints = NumberOfInts( size );

	TqInt i;
	for ( i = 0; i < numints; i++ )
		m_aBits[ i ] = ( m_aBits[ i ] & from.m_aBits[ i ] );

	Canonize();

	return ( *this );
}


//----------------------------------------------------------------------
/** Return a new bitvector which is the union of the current one and the one specified.
 * If they are of different sizes the result will be equal to the largest of the two.
 * \param from CqBitVector reference to perform the operation with.
 */

CqBitVector&	CqBitVector::Union( CqBitVector& from )
{
	TqInt size = ( from.m_cLength > m_cLength ) ? from.m_cLength : m_cLength;
	TqInt ssize = ( from.m_cLength < m_cLength ) ? from.m_cLength : m_cLength;
	SetSize( size );
	TqInt numints = NumberOfInts( ssize );

	TqInt i;
	for ( i = 0; i < numints; i++ )
		m_aBits[ i ] = ( m_aBits[ i ] | from.m_aBits[ i ] );

	Canonize();

	return ( *this );
}


//----------------------------------------------------------------------
/** Return a new bitvector which is the difference of the current one and the one specified.
 * If they are of different sizes the result will be equal to the largest of the two.
 * \param from CqBitVector reference to perform the operation with.
 */

CqBitVector&	CqBitVector::Difference( CqBitVector& from )
{
	TqInt size = ( from.m_cLength > m_cLength ) ? from.m_cLength : m_cLength;
	TqInt ssize = ( from.m_cLength < m_cLength ) ? from.m_cLength : m_cLength;
	SetSize( size );
	TqInt numints = NumberOfInts( ssize );

	TqInt i;
	for ( i = 0; i < numints; i++ )
		m_aBits[ i ] = ( m_aBits[ i ] ^ from.m_aBits[ i ] );

	Canonize();

	return ( *this );
}


//----------------------------------------------------------------------
/** Count the number of set bits in the bit vector.
 */

TqInt CqBitVector::Count() const
{
	register TqInt count;
	register TqInt i;

	static const unsigned bitcount[ 256 ] =
	    {
	        0, 1, 1, 2, 1, 2, 2, 3, 1, \
	        2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, \
	        4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, \
	        3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, \
	        3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, \
	        4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, \
	        5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, \
	        2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, \
	        4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, \
	        4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, \
	        6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, \
	        4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, \
	        5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, \
	        6, 6, 7, 6, 7, 7, 8
	    };

	for ( count = 0L, i = 0; i < m_cNumInts; i++ )
		count += bitcount[ m_aBits[ i ] ];
	return ( count );
}


//----------------------------------------------------------------------
/** Outputs a vector to an output stream.
 * \param Stream Stream to output the matrix to.
 * \param Vector The vector to output.
 * \return The new state of Stream.
 */

std::ostream &operator<<( std::ostream &Stream, CqBitVector &Vector )
{
	TqInt numints = Vector.ArraySize();
	Vector.Canonize();

	TqInt i;
	for ( i = 0; i < numints; i++ )
		Stream << std::hex << (int)Vector.IntArray()[i];
	return ( Stream );
}


} // namespace Aqsis
//---------------------------------------------------------------------
