// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
#include "exception.h"
#include "parameters.h"

USING_NAMESPACE( Aqsis );

static TqUlong huniform = CqString::hash( "uniform" );
static TqUlong hconstant = CqString::hash( "constant" );
static TqUlong hvarying = CqString::hash( "varying" );
static TqUlong hvertex = CqString::hash( "vertex" );
static TqUlong hfacevarying = CqString::hash( "facevarying" );
static TqUlong hfacevertex = CqString::hash( "facevertex" );

static TqUlong hfloat = CqString::hash( "float" );
static TqUlong hpoint = CqString::hash( "point" );
static TqUlong hhpoint = CqString::hash( "hpoint" );
static TqUlong hvector = CqString::hash( "vector" );
static TqUlong hnormal = CqString::hash( "normal" );
static TqUlong hcolor = CqString::hash( "color" );
static TqUlong hstring = CqString::hash( "string" );
static TqUlong hmatrix = CqString::hash( "matrix" );
static TqUlong hinteger = CqString::hash( "integer" );
static TqUlong hint = CqString::hash( "int" );

static TqUlong hleft = CqString::hash( "[" );
static TqUlong hright = CqString::hash( "]" );

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
			throw XqException ( "void parameter declaration" );
			case 4:
			case 7:
			throw XqException ( "Bad inline declaration" );
			return ;
			case 1:
			inline_def = false;
			break;
			case 2:
			lc( word[ 0 ] );
			if ( is_type( word[ 0 ] ) == false )
			{
				throw XqException ( "Bad inline declaration" );
			}
			inline_def = true;
			tc = class_uniform;
			tt = get_type( word[ 0 ] );
			size = 1;
			identifier = word[ 1 ];
			break;
			case 3:
			lc( word[ 0 ] );
			lc( word[ 1 ] );
			if ( ( is_class( word[ 0 ] ) == false ) || ( is_type( word[ 1 ] ) == false ) )
			{
				throw XqException ( "Bad inline declaration" );
			}
			inline_def = true;
			tc = get_class( word[ 0 ] );
			tt = get_type( word[ 1 ] );
			size = 1;
			identifier = word[ 2 ];
			break;
			case 5:
			lc( word[ 0 ] );
			if ( ( is_type( word[ 0 ] ) == false ) || ( word[ 1 ] != "[" ) ||
			        ( is_int( word[ 2 ] ) == false ) || ( word[ 3 ] != "]" ) )
			{
				throw XqException ( "Bad inline declaration" );
			}
			inline_def = true;
			tc = class_uniform;
			tt = get_type( word[ 0 ] );
			size = get_size( word[ 2 ] );
			identifier = word[ 4 ];
			break;
			case 6:
			lc( word[ 0 ] );
			lc( word[ 1 ] );
			if ( ( is_class( word[ 0 ] ) == false ) || ( is_type( word[ 1 ] ) == false ) ||
			        ( word[ 2 ] != "[" ) || ( is_int( word[ 3 ] ) == false ) ||
			        ( word[ 4 ] != "]" ) )
			{
				throw XqException ( "Bad inline declaration" );
			}
			inline_def = true;
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
	bool start_found;

	sp = 0;
	sz = 1;
	j = 0;
	start_found = false;

	for ( i = 0;( i < str.length() ) && ( j < 7 );i++ )
	{
		switch ( str[ i ] )
		{
				case ' ':
				case '\t':
				case '\n':
				if ( start_found == true )
				{
					word[ j ] = str.substr( sp, sz );
					j++;
					sz = 1;
				}
				start_found = false;
				break;
				case '#':
				throw XqException ( "'#' character not allowed in strings" );
				case '\"':
				throw XqException ( "'\"' character not allowed in strings" );
				case '[':
				case ']':
				if ( start_found == true )
				{
					word[ j ] = str.substr( sp, sz );
					j++;
					start_found = false;
				}
				sp = i;
				sz = 1;
				word[ j ] = str.substr( sp, sz );
				j++;
				break;
				default:
				if ( start_found == true )
				{
					sz += 1;
					break;
				}
				start_found = true;
				sp = i;
				sz = 1;
		}
	}
	// if there is no space at the end of the string,
	// the previous loop will not notice the end of the word,
	// and so will 'forget' to store it.
	if ( start_found == true )
	{
		word[ j ] = str.substr( sp, sz );
		j++;
	}
	number_of_words = j;
	try
	{
		check_syntax ();
	}
	catch(XqException e )
	{
		std::string strError( e.what () );
		strError += " : ";
		strError += str;
		throw( XqException( strError ) );
	}
}

bool CqInlineParse::is_class ( const std::string &str )
{
	TqUlong param = CqString::hash( str.c_str() );

	if ( ( param == hconstant ) ||
	        ( param == huniform ) ||
	        ( param == hvarying ) ||
	        ( param == hvertex ) ||
	        ( param == hfacevarying ) ||
	  	( param == hfacevertex ) )
		return true;
	return false;
}

bool CqInlineParse::is_type ( const std::string &str )
{
	TqUlong param = CqString::hash( str.c_str() );

	if ( ( param == hfloat ) ||
	        ( param == hpoint ) ||
	        ( param == hvector ) ||
	        ( param == hnormal ) ||
	        ( param == hcolor ) ||
	        ( param == hstring ) ||
	        ( param == hmatrix ) ||
	        ( param == hhpoint ) ||
	        ( param == hinteger ) ||
	        ( param == hint ) )
		return true;
	return false;
}

// check if this int is >0 too
bool CqInlineParse::is_int ( const std::string &str )
{
	int i, j;
	i = sscanf( str.c_str(), "%d", &j );
	if ( ( i != 1 ) || ( j <= 0 ) )
		return false;
	return true;
}

EqVariableClass CqInlineParse::get_class ( const std::string &str )
{
	TqUlong param = CqString::hash( str.c_str() );

	if ( param == hconstant )
		return class_constant;
	if ( param == huniform )
		return class_uniform;
	if ( param == hvarying )
		return class_varying;
	if ( param == hvertex )
		return class_vertex;
	if ( param == hfacevarying )
		return class_facevarying;
	if ( param == hfacevertex )
		return class_facevertex;
	return ( class_constant );
}

EqVariableType CqInlineParse::get_type ( const std::string &str )
{
	TqUlong param = CqString::hash( str.c_str() );

	if ( param == hfloat )
		return type_float;
	if ( param == hpoint )
		return type_point;
	if ( param == hvector )
		return type_vector;
	if ( param == hnormal )
		return type_normal;
	if ( param == hcolor )
		return type_color;
	if ( param == hstring )
		return type_string;
	if ( param == hmatrix )
		return type_matrix;
	if ( param == hhpoint )
		return type_hpoint;
	if ( param == hinteger )
		return type_integer;
	if ( param == hint )
		return type_integer;

	return ( type_float );
}

TqUint CqInlineParse::get_size ( const std::string &str )
{
	TqUint i;
	i = atoi( str.c_str() );
	return i;
}

void CqInlineParse::lc( std::string &str )
{
	for ( TqUint i = 0;i < str.length();i++ )
	{
		str[ i ] = tolower( str[ i ] );
	}
}
