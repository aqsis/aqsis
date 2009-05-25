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
		\brief Declares an extended string class, CqString, derived from std::string
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is .h included already?
#ifndef SSTRING_H_INCLUDED
#define SSTRING_H_INCLUDED 1

#include <string>
#include <sstream>
#include <iostream>

#include <aqsis/aqsis.h>


namespace Aqsis {

typedef std::string CqStringBase;

//----------------------------------------------------------------------
/** \class CqString
 * An extended string class, derived from std::string
 */

class AQSIS_UTIL_SHARE CqString : public CqStringBase
{
	public:
		CqString() : CqStringBase()
		{}
		CqString( const CqString& str ) : CqStringBase( str )
		{}
		CqString( const CqStringBase& str ) : CqStringBase( str )
		{}
		CqString( const TqChar* s ) : CqStringBase( s )
		{}
		/** Construct from an integer,
		 */
		CqString( TqInt i )
		{
			*this += i;
		}
		/** Construct from a float,
		 */
		CqString( TqFloat f )
		{
			*this += f;
		}


		inline static TqUlong	hash( const char *strName )
		{
			TqUlong retval = 0;
			const char *p = strName;
			retval = *p;

			if(retval)
			{
				for (p += 1; *p != '\0'; p++)
				{
					retval = (retval << 5) - retval + *p;
				}
			}
			return retval;
		}

		// Format a string printf style.
		CqString&	Format( const TqChar* Format, ... );
		CqString	ExpandEscapes() const;
		CqString	TranslateEscapes() const;
		CqString	ToLower() const;

		// Concatenation functions not provided by std::string
		CqString&	operator+=( const CqString& str );
		CqString&	operator+=( const TqChar* str );
		CqString&	operator+=( TqChar c );
		CqString&	operator+=( TqInt i );
		CqString&	operator+=( TqFloat f );
};


// Some useful functions
AQSIS_UTIL_SHARE std::ostream& operator<<( std::ostream & stmOutput, const CqString& strString );
AQSIS_UTIL_SHARE CqString operator+( const CqString& strAdd1, const CqString& strAdd2 );
AQSIS_UTIL_SHARE CqString operator+( const TqChar* strAdd1, const CqString& strAdd2 );
AQSIS_UTIL_SHARE CqString operator+( const CqString& strAdd1, const TqChar* strAdd2 );
AQSIS_UTIL_SHARE CqString operator+( const CqString& strAdd1, TqChar ch );
AQSIS_UTIL_SHARE CqString operator+( TqChar ch, const CqString& strAdd2 );

// These must be defined so that std::string can be used as a type in the ShaderVM and
// in the parameter dicing code.
AQSIS_UTIL_SHARE CqString operator/( const CqString& strAdd1, const CqString& strAdd2 );
AQSIS_UTIL_SHARE CqString operator*( const CqString& strAdd1, const CqString& strAdd2 );
AQSIS_UTIL_SHARE CqString operator*( const CqString& strAdd1, TqFloat f );
AQSIS_UTIL_SHARE CqString operator-( const CqString& strAdd1, const CqString& strAdd2 );

/// The ultimate function for converting anything into a string
template<typename value_t>
CqString ToString(const value_t& Value)
{
	std::ostringstream buffer;
	buffer << Value;
	return CqString(buffer.str());
}

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !SSTRING_H_INCLUDED


