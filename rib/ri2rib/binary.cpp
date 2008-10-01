// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
//
// -------------------------------------------------------------------------
//           The RenderMan (R) Interface Procedures and Protocol are:
//                     Copyright 1988, 1989, 2000, Pixar
//                           All rights reserved
// -------------------------------------------------------------------------

/** \file
 *  \brief Binary encoding output class implementation.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#if _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "binary.h"
#include <iostream>
#include <string>

#define OUT (*out)

namespace libri2rib {


void CqBinary::intToChar( RtInt n, TqChar &b1, TqChar &b2, TqChar &b3, TqChar &b4 )
{
	b1 = ( n >> 24 ) & 0xff;
	b2 = ( n >> 16 ) & 0xff;
	b3 = ( n >> 8 ) & 0xff;
	b4 = n & 0xff;
}

void CqBinary::floatToChar( RtFloat f, TqChar &b1, TqChar &b2, TqChar &b3, TqChar &b4 )
{
	TqPchar g;
	g = reinterpret_cast<TqPchar>( &f );
#ifdef WORDS_BIGENDIAN

	b1 = g[ 0 ];
	b2 = g[ 1 ];
	b3 = g[ 2 ];
	b4 = g[ 3 ];
#else

	b1 = g[ 3 ];
	b2 = g[ 2 ];
	b3 = g[ 1 ];
	b4 = g[ 0 ];
#endif
}

void CqBinary::addString( const std::string &s, bool &defined, TqUint &index )
{
	TqUint j = 0;
	for ( std::list<std::string>::iterator it = m_aStrings.begin(); it != m_aStrings.end(); it++, j++ )
	{
		if ( s == *it )
		{
			defined = true;
			index = j;
			return ;
		}
	}
	if ( j >= 65536 )
		return ;

	m_aStrings.push_back( s );

	defined = false;
	index = j;
}

void CqBinary::encodeString( const char *s )
{
	std::string str( s );
	TqChar b1, b2, b3, b4;
	TqUint sz = str.length();

	if ( sz < 16 )
	{
		OUT << ( TqChar ) ( sz + 0220 ); // 0x90
	}
	else
	{
		intToChar( sz, b1, b2, b3, b4 );
		if ( sz < 256 )
		{
			OUT << '\240' << b4; // 0xA0
		}
		else if ( sz < 256 * 256 )
		{
			OUT << '\241' << b3 << b4; // 0xA1
		}
		else if ( sz < 256 * 256 * 256 )
		{
			OUT << '\242' << b2 << b3 << b4; // 0xA2
		}
		else
		{
			OUT << '\243' << b1 << b2 << b3 << b4; // 0xA3
		}
	}
	for ( TqUint i = 0; i < sz; i++ )
		OUT << str[ i ];
}


CqBinary::CqBinary( const char *name, int fdesc,
                    const SqOptions::EqCompression comp)
		: CqOutput( name, fdesc, comp)
{
	for ( TqInt ii = 0; ii < LAST_Function; ii++ )
		m_aRequest[ ii ] = false;
}


CqBinary::~CqBinary()
{ }


void CqBinary::printHeader()
{
	OUT << "##RenderMan RIB-Structure 1.0\n";
	OUT << "version";
	// 3.03
	OUT << '\212' << '\003' << '\007' << '\256'; // 0x8A 0x03 0x07 0xAE
}

void CqBinary::printRequest( const char *req, EqFunctions f )
{
	TqChar code = f;

	if ( m_aRequest[ f ] == false )
	{
		m_aRequest[ f ] = true;
		OUT << '\314' << code; // 0xCC
		encodeString(req);
	}
	OUT << '\246' << code; // 0xA6
}

void CqBinary::printInteger( const RtInt i )
{
	TqChar b1, b2, b3, b4;
	intToChar( i, b1, b2, b3, b4 );

	RtInt abs = i;
	if ( abs < 0 )
		abs = -abs;

	if ( abs < 0x00000080 )
	{
		OUT << '\200' << b4; // 0x80
	}
	else if ( abs < 0x00008000 )
	{
		OUT << '\201' << b3 << b4; // 0x81
	}
	else if ( abs < 0x00800000 )
	{
		OUT << '\202' << b2 << b3 << b4; // 0x82
	}
	else
	{
		OUT << '\203' << b1 << b2 << b3 << b4; // 0x83
	}
}

void CqBinary::printFloat( const RtFloat f )
{
	TqChar b1, b2, b3, b4;
	floatToChar( f, b1, b2, b3, b4 );
	OUT << '\244' << b1 << b2 << b3 << b4; // 0xA4
}

void CqBinary::printString( const std::string &s )
{
	if ( ( s.length() <= 1 ) )
	{
		encodeString( s.c_str() );
		return ;
	}

	bool defined;
	TqUint index;

	addString( s, defined, index );

	if ( index < 256 )
	{
		if ( defined == false )
		{
			OUT << '\315' << ( TqChar ) index; // 0xCD
			encodeString( s.c_str() );
		}
		OUT << '\317' << ( TqChar ) index; // 0xCF
	}
	else if ( index < 65536 )
	{
		if ( defined == false )
		{
			OUT << '\316' << ( TqChar ) ( ( index >> 8 ) & 0xff ) << ( TqChar ) ( index & 0xff ); // 0xCE
			encodeString( s.c_str() );
		}
		OUT << '\320' << ( TqChar ) ( ( index >> 8 ) & 0xff ) << ( TqChar ) ( index & 0xff ); // 0xD0
	}
	else
	{
		encodeString( s.c_str() );
	}
}

void CqBinary::printArray ( RtInt n, RtInt *p )
{
	OUT << '[';
	for ( TqInt i = 0; i < n;i++ )
		printInteger( p[ i ] );
	OUT << ']';
}

void CqBinary::printArray ( RtInt n, RtFloat *p )
{
	TqChar b1, b2, b3, b4;
	intToChar( n, b1, b2, b3, b4 );

	if ( ( n & 0xFFFFFF00 ) == 0 )
	{
		OUT << '\310' << b4; // 0xC8
	}
	else if ( ( n & 0xFFFF0000 ) == 0 )
	{
		OUT << '\311' << b3 << b4; // 0xC9
	}
	else if ( ( n & 0xFF000000 ) == 0 )
	{
		OUT << '\312' << b2 << b3 << b4; // 0xCA
	}
	else
	{
		OUT << '\313' << b1 << b2 << b3 << b4; // 0xCB
	}

	for ( TqInt i = 0; i < n;i++ )
	{
		floatToChar( p[ i ], b1, b2, b3, b4 );
		OUT << b1 << b2 << b3 << b4;
	}
}

void CqBinary::printArray ( RtInt n, RtToken *p )
{
	OUT << '[';
	for ( TqInt i = 0; i < n;i++ )
		printToken( p[ i ] );
	OUT << ']';
}

void CqBinary::printCharP ( const char *c )
{
	std::string s( c );
	printString( s );
}

void CqBinary::printToken ( RtToken t )
{
	std::string s( t );
	printString( s );
}

void CqBinary::print ( const char *c )
{
	std::string str( c );
	OUT << str;
}

} // namespace libri2rib
