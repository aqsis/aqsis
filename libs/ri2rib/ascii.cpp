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

/** \file
 *  \brief RIB ascii output class implementation.
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#if _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "ascii.h"

#define OUT (*out)

namespace libri2rib {

CqASCII::CqASCII(const char *name, const int fdesc,
			const SqOptions::EqCompression comp,
			const SqOptions::EqIndentation i, const TqInt isize )
		: CqOutput( name, fdesc, comp ),
		m_Indentation( i ),
		m_IndentSize( isize ),
		m_IndentLevel( 0 )
{}

CqASCII::~CqASCII()
{}


// **************************************************************
// ******* ******* ******* BLOCK NESTING ******** ******* *******
// **************************************************************

void CqASCII::beginNesting(EqBlocks type)
{
	CqOutput::beginNesting(type);
	if(type != B_Ri)
		++m_IndentLevel;
}

void CqASCII::endNesting(EqBlocks type)
{
	if(type != B_Ri && m_IndentLevel > 0)
		--m_IndentLevel;
	CqOutput::endNesting(type);
	// The above may throw, and our indenting will be screwed up.  We could
	// catch this and try to recover, but such a RIB is broken anyway.
}

// **************************************************************
// ******* ******* ******* PRINTING TOOLS ******* ******* *******
// **************************************************************
void CqASCII::printHeader()
{
	OUT << "##RenderMan RIB-Structure 1.0\n";
	OUT << "version 3.03\n";
}

void CqASCII::printIndentation()
{
	switch ( m_Indentation )
	{
		case SqOptions::Indentation_None:
		break;
		case SqOptions::Indentation_Space:
			for (TqInt i = 0; i < m_IndentSize * m_IndentLevel; i++ )
				OUT << ' ';
			break;
		case SqOptions::Indentation_Tab:
			for (TqInt i = 0; i < m_IndentSize * m_IndentLevel; i++ )
				OUT << '\t';
			break;
	}
}

void CqASCII::printRequest( const char *cp, EqFunctions )
{
	printIndentation();
	OUT << cp;
}

void CqASCII::printInteger( const RtInt i )
{
	OUT << i;
}

void CqASCII::printFloat( const RtFloat f )
{
	if (f == RI_INFINITY)
	{
		OUT << "1e38";
	} else
	{
		OUT << f;
	}
}

void CqASCII::printString( const std::string &str )
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

const RtInt maxLineLength = 10;
void CqASCII::printArray ( RtInt n, RtInt *p )
{
	OUT << "[ ";
	for ( RtInt i = 0; i < n; i++ )
	{
		OUT << p[ i ] << ' ';
		//if ( (i+1) % maxLineLength == 0 )
			//printEOL();
	}
	OUT << ']';
}

void CqASCII::printArray ( RtInt n, RtFloat *p )
{
	OUT << "[ ";
	for ( RtInt i = 0; i < n; i++ )
	{
		OUT << p[ i ] << ' ';
		//if ( (i+1) % maxLineLength == 0 )
			//printEOL();
	}
	OUT << ']';
}


void CqASCII::printArray ( RtInt n, RtToken *p )
{
	OUT << "[ ";
	for ( RtInt i = 0; i < n; i++ )
	{
		printToken(p[ i ]);
		OUT << ' ';
		//if ( (i+1) % maxLineLength == 0 )
			//printEOL();
	}
	OUT << ']';
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


} // namespace libri2rib
