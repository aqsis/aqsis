// Aqsis
// Copyright  1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
 *  \brief A class to parse RiDeclare and inline parameter list definitions.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */
#include "inlineparse.h"

USING_NAMESPACE( libri2rib );

void CqInlineParse::check_syntax ()
{
	// number_of_words =0 ---> ERROR
	// number_of_words =1 ---> not an inline def
	// number_of_words =2 ---> type id
	// number_of_words =3 ---> class type id
	// number_of_words =4 ---> ERROR
	// number_of_words =5 ---> type [ size ] id
	// number_of_words =6 ---> class type [ size ] id
	// number_of_words =7 ---> ERROR

	switch ( number_of_words )
	{
			case 0:
			throw CqError ( RIE_SYNTAX, RIE_ERROR, "void parameter declaration", TqFalse );
			case 4:
			case 7:
			throw CqError ( RIE_SYNTAX, RIE_ERROR, "Bad inline declaration", TqTrue );
			return ;
			case 1:
			inline_def = TqFalse;
			break;
			case 2:
			lc( word[ 0 ] );
			if ( is_type( word[ 0 ] ) == TqFalse )
			{
				throw CqError ( RIE_SYNTAX, RIE_ERROR, "Bad inline declaration", TqTrue );
			}
			inline_def = TqTrue;
			tc = UNIFORM;
			tt = get_type( word[ 0 ] );
			size = 1;
			identifier = word[ 1 ];
			break;
			case 3:
			lc( word[ 0 ] );
			lc( word[ 1 ] );
			if ( ( is_class( word[ 0 ] ) == TqFalse ) || ( is_type( word[ 1 ] ) == TqFalse ) )
			{
				throw CqError ( RIE_SYNTAX, RIE_ERROR, "Bad inline declaration", TqTrue );
			}
			inline_def = TqTrue;
			tc = get_class( word[ 0 ] );
			tt = get_type( word[ 1 ] );
			size = 1;
			identifier = word[ 2 ];
			break;
			case 5:
			lc( word[ 0 ] );
			if ( ( is_type( word[ 0 ] ) == TqFalse ) || ( word[ 1 ] != "[" ) ||
			        ( is_int( word[ 2 ] ) == TqFalse ) || ( word[ 3 ] != "]" ) )
			{
				throw CqError ( RIE_SYNTAX, RIE_ERROR, "Bad inline declaration", TqTrue );
			}
			inline_def = TqTrue;
			tc = UNIFORM;
			tt = get_type( word[ 0 ] );
			size = get_size( word[ 2 ] );
			identifier = word[ 4 ];
			break;
			case 6:
			lc( word[ 0 ] );
			lc( word[ 1 ] );
			if ( ( is_class( word[ 0 ] ) == TqFalse ) || ( is_type( word[ 1 ] ) == TqFalse ) ||
			        ( word[ 2 ] != "[" ) || ( is_int( word[ 3 ] ) == TqFalse ) ||
			        ( word[ 4 ] != "]" ) )
			{
				throw CqError( RIE_SYNTAX, RIE_ERROR, "Bad inline declaration", TqTrue );
			}
			inline_def = TqTrue;
			tc = get_class( word[ 0 ] );
			tt = get_type( word[ 1 ] );
			size = get_size( word[ 3 ] );
			identifier = word[ 5 ];
			break;
	}
}

void CqInlineParse::parse ( std::string &str )
{
	TqUint i, j;
	size_t sp;
	size_t sz;
	TqBool start_found;

	sp = 0;
	sz = 1;
	j = 0;
	start_found = TqFalse;

	for ( i = 0;( i < str.length() ) && ( j < 7 );i++ )
	{
		switch ( str[ i ] )
		{
				case ' ':
				case '\t':
				case '\n':
				if ( start_found == TqTrue )
				{
					word[ j ] = str.substr( sp, sz );
					j++;
					sz = 1;
				}
				start_found = TqFalse;
				break;
				case '#':
				throw CqError( RIE_SYNTAX, RIE_ERROR, "'#' character not allowed in strings", TqTrue );
				case '\"':
				throw CqError( RIE_SYNTAX, RIE_ERROR, "'\"' character not allowed in strings", TqTrue );
				case '[':
				case ']':
				if ( start_found == TqTrue )
				{
					word[ j ] = str.substr( sp, sz );
					j++;
					start_found = TqFalse;
				}
				sp = i;
				sz = 1;
				word[ j ] = str.substr( sp, sz );
				j++;
				break;
				default:
				if ( start_found == TqTrue )
				{
					sz += 1;
					break;
				}
				start_found = TqTrue;
				sp = i;
				sz = 1;
		}
	}
	// if there is no space at the end of the string,
	// the previous loop will not notice the end of the word,
	// and so will 'forget' to store it.
	if ( start_found == TqTrue )
	{
		word[ j ] = str.substr( sp, sz );
		j++;
	}
	number_of_words = j;
	check_syntax ();
}

TqBool CqInlineParse::is_class ( const std::string &str )
{
	if ( ( str == "constant" ) ||
	        ( str == "uniform" ) ||
	        ( str == "varying" ) ||
	        ( str == "vertex" )  ||
	        ( str == "facevarying") )
		return TqTrue;
	return TqFalse;
}

TqBool CqInlineParse::is_type ( const std::string &str )
{
	if ( ( str == "float" ) ||
	        ( str == "point" ) ||
	        ( str == "vector" ) ||
	        ( str == "normal" ) ||
	        ( str == "color" ) ||
	        ( str == "string" ) ||
	        ( str == "matrix" ) ||
	        ( str == "hpoint" ) ||
	        ( str == "integer" ) ||
	        ( str == "int" ) )
		return TqTrue;
	return TqFalse;
}

// check if this int is >0 too
TqBool CqInlineParse::is_int ( const std::string &str )
{
	int i, j;
	i = sscanf( str.c_str(), "%d", &j );
	if ( ( i != 1 ) || ( j <= 0 ) )
		return TqFalse;
	return TqTrue;
}

EqTokenClass CqInlineParse::get_class ( const std::string &str )
{
	if ( str == "constant" )
		return CONSTANT;
	if ( str == "uniform" )
		return UNIFORM;
	if ( str == "varying" )
		return VARYING;
	if ( str == "vertex" )
		return VERTEX;
	if ( str == "facevarying" )
		return FACEVARYING;
	return ( CONSTANT );
}

EqTokenType CqInlineParse::get_type ( const std::string &str )
{
	if ( str == "float" )
		return FLOAT;
	if ( str == "point" )
		return POINT;
	if ( str == "vector" )
		return VECTOR;
	if ( str == "normal" )
		return NORMAL;
	if ( str == "color" )
		return COLOR;
	if ( str == "string" )
		return STRING;
	if ( str == "matrix" )
		return MATRIX;
	if ( str == "hpoint" )
		return HPOINT;
	if ( str == "integer" )
		return INTEGER;
	if ( str == "int" )
		return INTEGER;
	return ( FLOAT );
}

TqUint CqInlineParse::get_size ( const std::string &str )
{
	TqUint i;
	sscanf( str.c_str(), "%u", &i );
	return i;
}

void CqInlineParse::lc( std::string &str )
{
	for ( TqUint i = 0;i < str.length();i++ )
	{
		str[ i ] = tolower( str[ i ] );
	}
}
