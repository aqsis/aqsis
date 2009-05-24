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
		\brief Implements an extended string class, CqString, derived from std::string
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<stdio.h>

#include	<stdarg.h>

#include	<aqsis/aqsis.h>
#include	<aqsis/util/sstring.h>

namespace Aqsis {

//---------------------------------------------------------------------
/**	Expands characters which require escape sequences.
 * \return A string in which escape characters are expanded.
 */

CqString CqString::ExpandEscapes() const
{
	CqString	strResult;
	const TqChar*	str = c_str();

	if ( str != NULL )
	{
		TqChar c = *str++;

		while ( c != 0 )
		{
			if ( c >= ' ' )
			{
				switch ( c )
				{
						case '\\':
						strResult += '\\' + '\\';
						break;
						case '\'':
						strResult += '\\' + '\'';
						break;
						case '\"':
						strResult += '\\' + '\"';
						break;
						default:
						strResult += c;
						break;
				}
			}
			else
			{
				strResult += '\\';

				switch ( c )
				{
						case '\a':
						strResult += 'a';
						break;
						case '\b':
						strResult += 'b';
						break;
						case '\n':
						strResult += 'n';
						break;
						case '\r':
						strResult += 'r';
						break;
						case '\t':
						strResult += 't';
						break;
						case '\0':
						strResult += '0';
						break;

						default:
						{
							strResult += 'x';
							TqInt i;
							for ( i = 0;i < 4;i++ )
							{
								TqInt Value = ( ( c >> 12 ) & 0x0F );	// Get high nibble;
								c <<= 4;

								if ( Value <= 9 )
									strResult += '0' + Value;
								else
									strResult += 'a' + Value;
							}
						}
						break;
				}
			}
			c = *str++;
		}
	}
	return ( strResult );
}


//---------------------------------------------------------------------
/**	Translates escape sequences into their actual characters.
 * \return A string in which escape characters are translated.
 */

CqString CqString::TranslateEscapes() const
{
	CqString	strResult;
	const TqChar*	str = c_str();

	if ( str != NULL )
	{
		TqChar c = *str++;

		while ( c != 0 )
		{
			if ( c == '\\' )
			{
				c = *str++;

				switch ( c )
				{
						case 'a':
						strResult += '\a';
						break;
						case 'b':
						strResult += '\b';
						break;
						case 'n':
						strResult += '\n';
						break;
						case 'r':
						strResult += '\r';
						break;
						case 't':
						strResult += '\t';
						break;

						case '0':
						{
							TqChar Value = 0;

							while ( ( c >= '0' && c <= '7' ) )
							{
								Value = Value * 8 + c - '0';
								c = *str++;
							}
							strResult += Value;
							str--;
						}
						break;

						case 'x':
						{
							TqChar Value = 0;

							c = *str++;

							while ( ( c >= '0' && c <= '9' ) || ( c >= 'A' && c <= 'F' ) || ( c >= 'a' && c <= 'f' ) )
							{
								Value *= 16;
								if ( c >= '0' && c <= '9' )
									Value += c - '0';
								else
									if ( c >= 'A' && c <= 'F' )
										Value += c - 'A' + 10;
									else
										if ( c >= 'a' && c <= 'f' )
											Value += c - 'a' + 10;

								c = *str++;
							}
							strResult += Value;
							str--;
						}
						break;

						default:
						strResult += c;
						break;
				}
			}
			else
				strResult += c;

			c = *str++;
		}
	}

	return ( strResult );
}


//---------------------------------------------------------------------
/** Format a string printf style
 */

CqString& CqString::Format( const TqChar* strFmt, ... )
{
	va_list marker;
	va_start( marker, strFmt );

	*this = "";

	TqInt i = 0;
	while ( strFmt[ i ] != '\0' )
	{
		switch ( strFmt[ i ] )
		{
				case '%':
				{
					i++;
					switch ( strFmt[ i ] )
					{
							case 'f':
							{
								TqFloat val = static_cast<TqFloat>( va_arg( marker, double ) );
								*this += ToString(val);
							}
							break;

							case 'd':
							case 'i':
							{
								TqInt val = va_arg( marker, TqInt );
								*this += ToString(val);
							}
							break;

							case 'x':
							{
								TqInt val = va_arg( marker, TqInt );
								*this += ToString(val);
							}
							break;

							case 's':
							*this += va_arg( marker, TqChar* );
							break;
					}
				}
				break;

				default:
				*this += strFmt[ i ];
				break;
		}
		i++;
	}

	va_end( marker );
	return ( *this );
}

CqString CqString::ToLower() const
{
	CqString res(*this);
	CqStringBase::iterator i;
	for(i=res.begin(); i!=res.end(); ++i)
		*i = tolower(*i);
	return(res);
}

CqString& CqString::operator+=( const CqString& str )
{
	CqStringBase::operator+=( str );
	return ( *this );
}

CqString& CqString::operator+=( const TqChar* str )
{
	CqStringBase::operator+=( str );
	return ( *this );
}

CqString& CqString::operator+=( TqChar c )
{
	CqStringBase::operator+=( c );
	return ( *this );
}

CqString& CqString::operator+=( TqInt i )
{
	*this += ToString(i);
	return *this;
}

CqString& CqString::operator+=( TqFloat f )
{
	*this += ToString(f);
	return *this;
}


//---------------------------------------------------------------------
/** Concatenate two strings
 */

CqString operator+( const CqString& strAdd1, const CqString& strAdd2 )
{
	CqString strRes( strAdd1 );
	strRes += strAdd2;
	return ( strRes );
}


//---------------------------------------------------------------------
/** Concatenate two strings
 */

CqString operator+( const TqChar* strAdd1, const CqString& strAdd2 )
{
	CqString strRes( strAdd1 );
	strRes += strAdd2;
	return ( strRes );
}


//---------------------------------------------------------------------
/** Concatenate two strings
 */

CqString operator+( const CqString& strAdd1, const TqChar* strAdd2 )
{
	CqString strRes( strAdd1 );
	strRes += strAdd2;
	return ( strRes );
}


//---------------------------------------------------------------------
/** Append charater to end of string
 */

CqString operator+( const CqString& strAdd1, TqChar ch )
{
	CqString strRes( strAdd1 );
	strRes += ch;
	return ( strRes );
}


//---------------------------------------------------------------------
/** Prepend character to string
 */

CqString operator+( TqChar ch, const CqString& strAdd2 )
{
	CqString strRes( ch );
	strRes += strAdd2;
	return ( strRes );
}


//---------------------------------------------------------------------
/** Not really useful but needed for the templatised operation of the render engine.
 */

CqString operator-( const CqString& strAdd1, const CqString& strAdd2 )
{
	return ( strAdd1 );
}


//---------------------------------------------------------------------
/** Not really useful but needed for the templatised operation of the render engine.
 */

CqString operator/( const CqString&strAdd1, const CqString&strAdd2 )
{
	return ( strAdd1 );
}


//---------------------------------------------------------------------
/** Not really useful but needed for the templatised operation of the render engine.
 */

CqString operator*( const CqString& strAdd1, const CqString& strAdd2 )
{
	return ( strAdd1 );
}


//---------------------------------------------------------------------
/** Not really useful but needed for the templatised operation of the render engine.
 */

CqString operator*( const CqString& strAdd1, TqFloat f )
{
	return ( strAdd1 );
}


std::ostream& operator<<( std::ostream & stmOutput, const CqString& strString )
{
	stmOutput << strString.c_str();
	return ( stmOutput );
}



//---------------------------------------------------------------------

} // namespace Aqsis
