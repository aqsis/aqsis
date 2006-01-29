// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
 *  \brief RIB ascii output class implementation.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifdef	WIN32
#pragma warning(disable : 4786)
#endif

#include "ascii.h"

#define OUT (*out)

USING_NAMESPACE( libri2rib )



// **************************************************************
// ******* ******* ******* PRINTING TOOLS ******* ******* *******
// **************************************************************
void CqASCII::printHeader()
{
	OUT << "##RenderMan RIB-Structure 1.0\n";
	OUT << "version 3.03\n";
}

void CqASCII::printRequest( const char *cp, EqFunctions )
{
	TqInt i;
	switch ( m_Indentation )
	{
			case SqOptions::Indentation_None:
			break;
			case SqOptions::Indentation_Space:
			for ( i = 0; i < m_IndentSize * m_IndentLevel; i++ )
				OUT << ' ';
			break;
			case SqOptions::Indentation_Tab:
			for ( i = 0; i < m_IndentSize * m_IndentLevel; i++ )
				OUT << '\t';
			break;
	}
	OUT << cp;
}

void CqASCII::printInteger( const RtInt i )
{
	OUT << i;
}

void CqASCII::printFloat( const RtFloat f )
{
	OUT << f;
}

void CqASCII::printString( std::string &str )
{
	OUT << '"' << str << '"';
}

void CqASCII::printSpace()
{
	OUT << ' ';
}

void CqASCII::printEOL()
{
	OUT << "\n";
}

void CqASCII::printArray ( RtInt n, RtInt *p )
{
	//    if( n > 0 )
	{
		OUT << "[ ";
		for ( RtInt i = 0; i < n; i++ )
		{
			OUT << p[ i ] << ' ';
		}
		OUT << ']';
	}
}

void CqASCII::printArray ( RtInt n, RtFloat *p )
{
	//    if( n > 0 )
	{
		OUT << "[ ";
		for ( RtInt i = 0; i < n; i++ )
		{
			OUT << p[ i ] << ' ';
		}
		OUT << ']';
	}
}


void CqASCII::printArray ( RtInt n, RtToken *p )
{
	//    if( n > 0 )
	{
		OUT << "[ ";
		for ( RtInt i = 0; i < n; i++ )
		{
			printToken(p[ i ]);
			OUT << ' ';
		}
		OUT << ']';
	}
}


void CqASCII::printToken( RtToken t )
{
	OUT << '"' << t << '"';
}

void CqASCII::printCharP( const char *c )
{
	OUT << '"' << c << '"';
}

void CqASCII::print( const char *c )
{
	OUT << c;
}
