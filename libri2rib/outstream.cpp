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
 *  \brief Fstream and Gzip output
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#ifdef	WIN32
#include	<io.h>
#endif
#include "outstream.h"
#include "errno.h"
#include "string.h"
#include "error.h"
#include <unistd.h>

USING_NAMESPACE( libri2rib );


void CqStreamGzip::error()
{
	int * e = 0;
	const char *cp = gzerror( gzf, e );

	if ( *e == Z_ERRNO )
	{
		throw CqError ( RIE_SYSTEM, RIE_ERROR, strerror( errno ), TqFalse );
	}
	else
	{
		throw CqError( RIE_SYSTEM, RIE_ERROR, cp, TqFalse );
	}
}

CqStream & CqStreamGzip::operator<< ( int i )
{
	if ( gzprintf( gzf, "%i", i ) == 0 ) error();
	return *this;
}

CqStream & CqStreamGzip::operator<< ( float f )
{
	if ( gzprintf( gzf, "%f", f ) == 0 ) error();
	return *this;
}

CqStream & CqStreamGzip::operator<< ( std::string s )
{
	if ( gzputs( gzf, s.c_str() ) == -1 ) error();
	return *this;
}

CqStream & CqStreamGzip::operator<< ( char c )
{
	if ( gzputc( gzf, c ) == -1 ) error();
	return *this;
}

void CqStreamGzip::openFile( const char *name )
{
	gzf = gzopen( name, "wb" );
	if ( gzf == NULL )
	{
		throw CqError( RIE_NOFILE, RIE_ERROR, "Unable to open file ", name, "", TqFalse );
	}
	gzsetparams( gzf, Z_DEFAULT_COMPRESSION, Z_DEFAULT_STRATEGY );
}

void CqStreamGzip::openFile( int fdesc )
{
	gzf = gzdopen( dup(fdesc), "wb" );
	if ( gzf == NULL )
	{
		char c[ 100 ];
		sprintf( c, "%u", fdesc );
		throw CqError( RIE_NOFILE, RIE_ERROR, "Unable to open file with descriptor=", c, "", TqFalse );
	}
}

void CqStreamGzip::closeFile()
{
	if ( gzf )
		gzclose( gzf );
}





void CqStreamFDesc::error()
{
	throw CqError ( RIE_SYSTEM, RIE_ERROR, strerror( errno ), TqFalse );
}

CqStream & CqStreamFDesc::operator<< ( int i )
{
	*ofstr << i;
	if( ofstr->eof() )
		error();
	return *this;
}

CqStream & CqStreamFDesc::operator<< ( float f )
{
	*ofstr << f;
	if( ofstr->eof() )
		error();
	return *this;
}

CqStream & CqStreamFDesc::operator<< ( std::string s )
{
	*ofstr << s;
	if( ofstr->eof() )
		error();
	return *this;
}

CqStream & CqStreamFDesc::operator<< ( char c )
{
	*ofstr << c;
	if( ofstr->eof() )
		error();
	return *this;
}

void CqStreamFDesc::openFile( const char *name )
{
	ofstr->open( name, std::ios::binary | std::ios::out | std::ios::trunc );
	if ( !ofstr->is_open() )
	{
		throw CqError( RIE_NOFILE, RIE_ERROR, "Unable to open file ", name, "", TqFalse );
	}
}

void CqStreamFDesc::closeFile()
{
	if ( ofstr->is_open() )
		ofstr->close();
}

void CqStreamFDesc::openFile( int )
{
	throw CqError( RIE_NOFILE, RIE_ERROR, "File descriptors unusable with std::stream" , TqFalse );
}
