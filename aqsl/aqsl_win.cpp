// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
		\brief Simple console app. to combine slpp and aqslcomp.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	<iostream>
#include	<strstream>

#ifdef AQSIS_SYSTEM_MACOSX
#include	<string.h>
#endif /* AQSIS_SYSTEM_MACOSX */


/** Compile the give sl file into an slx bytestream.
 */
void compile_file( const char* sl_file )
{
	FILE * hPipeRead;
	FILE* hPipeWrite;

	std::strstream slppcommand;

#ifdef AQSIS_SYSTEM_MACOSX
	slppcommand << "${AQSIS_BASE_PATH}/slpp -d PI=3.141592654 -d AQSIS -c6 " << sl_file << std::ends;
	hPipeRead = popen( slppcommand.str(), "r" );
	hPipeWrite = popen( "${AQSIS_BASE_PATH}/aqslcomp", "w" );
#else
	slppcommand << "slpp.exe -d PI=3.141592654 -d AQSIS -c6 " << sl_file << std::ends;
	hPipeRead = _popen( slppcommand.str(), "r" );
	hPipeWrite = _popen( "aqslcomp.exe", "w" );
#endif /* AQSIS_SYSTEM_MACOSX */

	char psBuffer[ 128 ];
	while ( !feof( hPipeRead ) )
	{
		if ( fgets( psBuffer, 128, hPipeRead ) != NULL )
			fputs( psBuffer, hPipeWrite );
	}

#ifdef AQSIS_SYSTEM_MACOSX
	pclose( hPipeRead );
	pclose( hPipeWrite );
#else
	_pclose( hPipeRead );
	_pclose( hPipeWrite );
#endif /* AQSIS_SYSTEM_MACOSX */

}


/** The main loop prcesses any arguments into a single string
 *  looking for -help, and then passes them directly onto the compiler command.
 */
int main( int argc, char** argv )
{
	int i;
	char* pargs = new char[255];
	pargs[0] = '\0';
	for ( i = 1; i < argc; i++ )
	{
		if ( strstr( argv[ i ], "-help" ) )
		{
			printf( "Usage: %s [options] yourshader.sl\n", argv[ 0 ] );
			printf( "all options will be passed directly to slpp\n" );
			exit( 2 );
		}
		strcat( pargs, argv[ i] );
		strcat( pargs, " " );
	}
	compile_file( pargs );

	delete[](pargs);
	return ( 0 );
}
