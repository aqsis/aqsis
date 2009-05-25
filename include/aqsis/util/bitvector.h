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
		\brief Declares the CqBitVector class for handling efficient bit vectors of any length.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef BITVECTOR_H_INCLUDED
#define BITVECTOR_H_INCLUDED 1

#include	<iostream>

#include	<aqsis/aqsis.h>

namespace Aqsis {

typedef unsigned char	bit;

/// Define the number of bits in a char
#ifndef CHAR_BIT
#define	CHAR_BIT	8
#endif

//----------------------------------------------------------------------
/**
	\brief Varying length bitvectors.
	Provide the functionality of any length bitvectors and logic operations between them.
 
*/

class AQSIS_UTIL_SHARE CqBitVector
{
	public:
		/** Default constructor.
		 * \param size the initial size of the bit vector.
		 */
		CqBitVector( TqInt size = 0 ) : m_aBits( 0 ), m_cLength( 0 ), m_cNumInts( 0 )
		{
			if ( size > 0 )
				SetSize( size );
		}
		/** Copy constructor.
		 * \param from the bitvector to copy.
		 */
		CqBitVector( const CqBitVector& from ) : m_aBits( 0 ), m_cLength( 0 ), m_cNumInts( 0 )
		{
			*this = from;
		}
		~CqBitVector()
		{
			delete[] ( m_aBits );
		}

		/** Get the size of the bit vector.
		 * \return integer size.
		 */
		TqInt	Size() const
		{
			return ( m_cLength );
		}
		/** Set the size of the bitvector.
		 * \warning If the bit vector grows, the contents of any additional bits is undefined.
		 * \param size the new size of the bit vector.
		 */
		void	SetSize( TqInt size )
		{
			TqInt cNumInts = NumberOfInts( size );
			if ( m_cNumInts != cNumInts )
			{
				delete[] ( m_aBits );
				m_cNumInts = NumberOfInts( size );
				m_aBits = new bit[ m_cNumInts ];
			}
			m_cLength = size;
		}

		/** Force the array om canonical form, i.e. all unused bits in the char array are zeroed.
		 */
		void	Canonize()
		{
			( m_aBits ) [ m_cNumInts - 1 ] &= ( bit ) ~0 >> ( CHAR_BIT - ( ( m_cLength % CHAR_BIT )
			                                  ? ( m_cLength % CHAR_BIT )
			                                  : CHAR_BIT ) );
		}
		/** Set the indexed bit to the boolean value specified.
		 * \param elem the index of the bit to modify.
		 * \param value the new value of the bit, 0-false, 1-true.
		 */
		void	SetValue( TqInt elem, bool value )
		{
			assert( elem < m_cLength );
			if ( value )
				m_aBits[ elem / CHAR_BIT ] |= ( 1 << ( elem % CHAR_BIT ) );
			else
				m_aBits[ elem / CHAR_BIT ] &= ~( 1 << ( elem % CHAR_BIT ) );
		}
		/** Get the indexed bit as a boolean.
		 * \param elem the index of the bit to retrieve.
		 */
		bool Value( TqInt elem ) const
		{
			assert( elem < m_cLength );
			return ( ( m_aBits[ elem / CHAR_BIT ] & ( 1 << ( elem % CHAR_BIT ) ) ) ? true : false );
		}
		/** Toggle the state of the indexed bit.
		 * \param elem the index of the bit to modify.
		 */
		void	Toggle( TqInt elem )
		{
			assert( elem < m_cLength );
			m_aBits[ elem / CHAR_BIT ] ^= ( 1 << ( elem % CHAR_BIT ) );
		}
		/** Set all bits to the specified value.
		 * \param value the new value of the bit, 0-false, 1-true.
		 */
		void	SetAll( bool value )
		{
			bit setval = ( value ) ? ~0 : 0;
			register TqInt i;

			for ( i = 0; i < m_cNumInts; i++ )
				m_aBits[ i ] = setval;
			Canonize();
		}
		/** Invert the state of all bits in the vector.
		 */
		void	Complement()
		{
			register TqInt i;

			for ( i = 0; i < m_cNumInts; i++ )
				m_aBits[ i ] = ~m_aBits[ i ];
			Canonize();
		}
		/// Count the number of 1 bits in the vector.
		TqInt Count() const;
		/// Boolean intersection.
		CqBitVector&	Intersect( CqBitVector& from );
		/// Boolean union.
		CqBitVector&	Union( CqBitVector& from );
		/// Boolean difference.
		CqBitVector&	Difference( CqBitVector& from );
		/** Assignment operator.
		 * \param from the bitvector to copy.
		 * \return a reference to this bit vector.
		 */
		CqBitVector& operator=( const CqBitVector& from )
		{
			// Copy the array of bits
			SetSize( from.m_cLength );
			for ( TqInt i = 0; i < m_cNumInts; i++ )
				m_aBits[ i ] = from.m_aBits[ i ];

			return ( *this );
		}
		/** Perform a bitwise AND on this with the specified bitvector.
		 * \param from the bitvector to perform the AND with.
		 * \return the result of the AND operation as a new bitvector.
		 */
		CqBitVector operator&( CqBitVector& from ) const
		{
			CqBitVector res( *this );
			res.Intersect( from );
			return ( res );
		}
		/** Perform a bitwise OR on this with the specified bitvector.
		 * \param from the bitvector to perform the OR with.
		 * \return the result of the OR operation as a new bitvector.
		 */
		CqBitVector operator|( CqBitVector& from ) const
		{
			CqBitVector res( *this );
			res.Union( from );
			return ( res );
		}
		/** Perform a bitwise exclusive OR on this with the specified bitvector.
		 * \param from the bitvector to perform the exclusive OR with.
		 * \return the result of the exclusive OR operation as a new bitvector.
		 */
		CqBitVector operator^( CqBitVector& from ) const
		{
			CqBitVector res( *this );
			res.Difference( from );
			return ( res );
		}

		/** Perform a bitwise AND on this with the specified bitvector, storing the result in this.
		 * \param from the bitvector to perform the AND with.
		 * \return a reference to this bitvector.
		 */
		CqBitVector& operator&=( CqBitVector& from )
		{
			Intersect( from );
			return ( *this );
		}
		/** Perform a bitwise OR on this with the specified bitvector, storing the result in this.
		 * \param from the bitvector to perform the OR with.
		 * \return a reference to this bitvector.
		 */
		CqBitVector& operator|=( CqBitVector& from )
		{
			Union( from );
			return ( *this );
		}
		/** Perform a bitwise exclusive OR on this with the specified bitvector, storing the result in this.
		 * \param from the bitvector to perform the exclusive OR with.
		 * \return a reference to this bitvector.
		 */
		CqBitVector& operator^=( CqBitVector& from )
		{
			Difference( from );
			return ( *this );
		}

		/** Get the number of bytes required to represent the whole bitvector.
		 * \param size the required size of the bitvector.
		 * \return an integer count of bytes needed.
		 */
		TqInt ArraySize() const
		{
			return( NumberOfInts(m_cLength) );
		}

		/** Get a pointer to the ints representing the bitvector.
		 * \return a pointer to the char array.
		 */
		bit* IntArray()
		{
			return ( m_aBits );
		}
		/** Get the number of bytes required to represent the specified number of bits.
		 * \param size the required size of the bitvector.
		 * \return an integer count of bytes needed.
		 */
		static	TqInt	NumberOfInts( TqInt size )
		{
			return ( ( size + ( CHAR_BIT ) - 1 ) / ( CHAR_BIT ) );
		}
		friend std::ostream &operator<<( std::ostream &Stream, CqBitVector &Vector );
	private:
		bit*	m_aBits;			///< the array of bytes to store the bit vector.
		TqInt	m_cLength;			///< the size of the bitvector in bits.
		TqInt	m_cNumInts;			///< the size of the array in bytes.
}
;


//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !BITVECTOR_H_INCLUDED
