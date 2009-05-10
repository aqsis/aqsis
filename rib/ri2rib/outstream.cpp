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
 *  \brief Fstream and Gzip output
 *  \author Lionel J. Lacour (intuition01@online.fr)
 */

#include "outstream.h"

#ifdef AQSIS_SYSTEM_WIN32
#	include <io.h>
#else
#	include <unistd.h>
#endif

#include "errno.h"
#include "string.h"
#include "error.h"


namespace libri2rib {

void CqStreamGzip::error()
{
	int * e = 0;
	const char *cp = gzerror( gzf, e );

	if ( *e == Z_ERRNO )
	{
		throw CqError ( RIE_SYSTEM, RIE_ERROR, strerror( errno ), false );
	}
	else
	{
		throw CqError( RIE_SYSTEM, RIE_ERROR, cp, false );
	}
}

CqStream & CqStreamGzip::operator<< ( int i )
{
	if ( gzprintf( gzf, "%i", i ) == 0 )
		error();
	return *this;
}

CqStream & CqStreamGzip::operator<< ( float f )
{
	if ( gzprintf( gzf, "%f", f ) == 0 )
		error();
	return *this;
}

CqStream & CqStreamGzip::operator<< ( std::string s )
{
	if ( gzputs( gzf, s.c_str() ) == -1 )
		error();
	return *this;
}

CqStream & CqStreamGzip::operator<< ( char c )
{
	if ( gzputc( gzf, c ) == -1 )
		error();
	return *this;
}

void CqStreamGzip::openFile( const char *name )
{
	gzf = gzopen( name, "wb" );
	if ( gzf == NULL )
	{
		throw CqError( RIE_NOFILE, RIE_ERROR, "Unable to open file ", name, "", false );
	}
	gzsetparams( gzf, Z_DEFAULT_COMPRESSION, Z_DEFAULT_STRATEGY );
}

void CqStreamGzip::openFile( int fdesc )
{
	gzf = gzdopen( dup( fdesc ), "wb" );
	if ( gzf == NULL )
	{
		char c[ 100 ];
		sprintf( c, "%u", fdesc );
		throw CqError( RIE_NOFILE, RIE_ERROR, "Unable to open file with descriptor=", c, "", false );
	}
}

void CqStreamGzip::closeFile()
{
	if ( gzf )
		gzclose( gzf );
}

void CqStreamGzip::flushFile()
{
	//  At the end of a procedural RunProgram the
	// gzip internal state must be reset.
	if ( gzf )
	{
		gzflush( gzf, Z_FINISH );
	}
}




void CqStreamFDesc::error()
{
	throw CqError ( RIE_SYSTEM, RIE_ERROR, strerror( errno ), false );
}

CqStream & CqStreamFDesc::operator<< ( int i )
{
	if ( fprintf( fstr, "%i", i ) < 0 )
		error();
	return *this;
}

CqStream & CqStreamFDesc::operator<< ( float f )
{
	if ( fprintf( fstr, "%f", f ) < 0 )
		error();
	return *this;
}

CqStream & CqStreamFDesc::operator<< ( std::string s )
{
	if ( fputs( s.c_str(), fstr ) == EOF )
		error();
	return *this;
}

CqStream & CqStreamFDesc::operator<< ( char c )
{
	if ( fputc( c, fstr ) == EOF )
		error();
	return *this;
}

void CqStreamFDesc::openFile( const char *name )
{
	fstr = fopen( name, "wb" );
	if ( fstr == NULL )
	{
		throw CqError( RIE_NOFILE, RIE_ERROR, "Unable to open file ", name, "", false );
	}
}

void CqStreamFDesc::openFile ( int fdesc )
{
	fstr = fdopen ( dup( fdesc ), "wb" );
	if ( fstr == NULL )
	{
		char c[ 100 ];
		sprintf( c, "%u", fdesc );
		throw CqError( RIE_NOFILE, RIE_ERROR, "Unable to open file with descriptor=", c, "", false );
	}
}

void CqStreamFDesc::closeFile()
{
	if ( fstr )
		fclose( fstr );
}

void CqStreamFDesc::flushFile()
{
	if ( fstr )
		fflush( fstr );
}

} // namespace libri2rib
