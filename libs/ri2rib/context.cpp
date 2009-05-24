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
 *  \brief RiContext and Options parsing class
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#if _MSC_VER
#pragma warning( disable : 4786 )
#endif //WIN32

#include "context.h"
#include "ascii.h"
#include "binary.h"
#include "error.h"

#include <cstring>

namespace libri2rib {


CqContext::CqContext() :
		m_PipeHandleSet ( false ),
		m_PipeHandle ( 1 ),
		m_OutputType( SqOptions::OutputType_Ascii ),
		m_Compression( SqOptions::Compression_None ),
		m_Indentation( SqOptions::Indentation_None ),
		m_IndentSize( 0 )
{
	m_Active = ( CqOutput * ) RI_NULL;
	m_lContextHandle.push_back( m_Active );
}

void CqContext::addContext( RtToken name )
{
	if ( name == NULL )
	{
		if ( m_PipeHandleSet == false )
			m_PipeHandle = 1;
	}

	switch ( m_OutputType )
	{
			case SqOptions::OutputType_Ascii:
			m_Active = new CqASCII ( name, m_PipeHandle, m_Compression,
			                         m_Indentation, m_IndentSize );
			break;
			case SqOptions::OutputType_Binary:
			m_Active = new CqBinary ( name, m_PipeHandle, m_Compression);
			break;
	}

	m_lContextHandle.push_back( m_Active );
}

RtContextHandle CqContext::getContext()
{
	return ( RtContextHandle ) m_Active;
}

CqOutput & CqContext::current()
{
	if ( m_Active == ( ( CqOutput * ) RI_NULL ) )
	{
		throw CqError( RIE_BUG, RIE_SEVERE, "No active context", false );
	}
	return *m_Active;
}

void CqContext::switchTo( RtContextHandle ch )
{
	std::list<CqOutput *>::iterator first = m_lContextHandle.begin();
	CqOutput *r = ( CqOutput * ) ch;
	for ( ;first != m_lContextHandle.end();first++ )
	{
		if ( ( *first ) == r )
		{
			m_Active = r;
			return ;
		}
	}
	throw CqError ( RIE_BUG, RIE_SEVERE, "Invalid Context Handle", false );
}

void CqContext::removeCurrent()
{
	std::list<CqOutput *>::iterator first = m_lContextHandle.begin();

	for ( ;first != m_lContextHandle.end();first++ )
	{
		if ( ( *first ) == m_Active )
		{
			delete ( *first );
			m_lContextHandle.erase( first );
			m_Active = ( CqOutput * ) RI_NULL;
			return ;
		}
	}
}


// Available options:
// ------------------

//----------------------------------------------------------------------------------------------
// Name:                Token:          Parameter:                     Default value:
//----------------------------------------------------------------------------------------------
// RI2RIB_Output
//                      Type            "Ascii" | "Binary"             "Ascii"
//                      Compression      "None" | "Gzip"               "None"
//                      PipeHandle           integer                        1  (Standard output)
//
// RI2RIB_Indentation
//                      Type            "None" | "Space" | "Tab"       "None"
//                      Size                 integer                        0
//----------------------------------------------------------------------------------------------



void CqContext::parseOutputType( RtInt n, RtToken tokens[], RtPointer params[] )
{
	for ( RtInt i = 0; i < n; i++ )
	{

		try
		{
			if ( strcmp( tokens[ i ], "Type" ) == 0 )
			{
				if ( strcmp( ( ( char ** ) params[ i ] ) [ 0 ], "Ascii" ) == 0 )
					m_OutputType = SqOptions::OutputType_Ascii;
				else if ( strcmp( ( ( char ** ) params[ i ] ) [ 0 ], "Binary" ) == 0 )
					m_OutputType = SqOptions::OutputType_Binary;
				else
				{
					throw CqError( RIE_CONSISTENCY, RIE_WARNING,
					               "RiOption: Unrecognized Output Type parameter \"",
					               ( ( char ** ) params[ i ] ) [ 0 ],
					               "\"", false );
				}

			}
			else if ( strcmp( tokens[ i ], "Compression" ) == 0 )
			{
				if ( strcmp( ( ( char ** ) params[ i ] ) [ 0 ], "None" ) == 0 )
					m_Compression = SqOptions::Compression_None;
				else if ( strcmp( ( ( char ** ) params[ i ] ) [ 0 ], "Gzip" ) == 0 )
					m_Compression = SqOptions::Compression_Gzip;
				else
				{
					throw CqError( RIE_CONSISTENCY, RIE_WARNING,
					               "RiOption: Unrecognized Compression parameter \"",
					               ( ( char ** ) params[ i ] ) [ 0 ],
					               "\"", false );
				}

			}
			else if ( strcmp ( tokens[ i ], "PipeHandle" ) == 0 )
			{
				m_PipeHandleSet = true;
				m_PipeHandle = ( ( int * ) params[ i ] ) [ 0 ];
			}
			else
			{
				throw CqError( RIE_BADTOKEN, RIE_WARNING,
				               "RiOption: Unrecognized Output token \"",
				               tokens[ i ],
				               "\"", false );
			}

		}
		catch ( CqError & r )
		{
			r.manage();
			continue;
		}
	}
}

void CqContext::parseIndentation( RtInt n, RtToken tokens[], RtPointer params[] )
{
	for ( RtInt i = 0; i < n; i++ )
	{

		try
		{
			if ( strcmp( tokens[ i ], "Type" ) == 0 )
			{
				if ( strcmp( ( ( char ** ) params[ i ] ) [ 0 ], "None" ) == 0 )
					m_Indentation = SqOptions::Indentation_None;
				else if ( strcmp( ( ( char ** ) params[ i ] ) [ 0 ], "Space" ) == 0 )
					m_Indentation = SqOptions::Indentation_Space;
				else if ( strcmp( ( ( char ** ) params[ i ] ) [ 0 ], "Tab" ) == 0 )
					m_Indentation = SqOptions::Indentation_Tab;
				else
				{
					throw CqError( RIE_CONSISTENCY, RIE_WARNING,
					               "RiOption: Unrecognized Indentation Type parameter\"",
					               ( ( char ** ) params[ i ] ) [ 0 ],
					               "\"", false );
				}

			}
			else if ( strcmp( tokens[ i ], "Size" ) == 0 )
			{
				TqInt * ii = ( ( TqInt ** ) params ) [ i ];
				if ( *ii >= 0 )
					m_IndentSize = *ii;
				else
				{
					throw CqError( RIE_CONSISTENCY, RIE_WARNING,
					               "RiOption: Indentation size must be positive", false );
				}

			}
			else
			{
				throw CqError( RIE_BADTOKEN, RIE_WARNING,
				               "RiOption: Unrecognized Indentation token \"",
				               tokens[ i ],
				               "\"", false );
			}

		}
		catch ( CqError & r )
		{
			r.manage();
			continue;
		}
	}
}

void CqContext::parseOption( const char *name, RtInt n, RtToken tokens[], RtPointer params[] )
{
	if ( strcmp( name, "RI2RIB_Output" ) == 0 )
	{
		parseOutputType( n, tokens, params );
	}
	else if ( strcmp( name, "RI2RIB_Indentation" ) == 0 )
	{
		parseIndentation( n, tokens, params );
	}
	else
	{
		throw CqError( RIE_CONSISTENCY, RIE_WARNING,
		               "RiOption: Unknown Option name \"", name, "\"",
		               false );
	}
}

} // namespace libri2rib
