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
void compile_file( std::string slppargs, std::string aqslcompargs )
{
	FILE * hPipeRead;
	FILE* hPipeWrite;

	std::strstream slppcommand;
	std::strstream aqslcompcommand;

#ifdef AQSIS_SYSTEM_MACOSX
	slppcommand << "${AQSIS_BASE_PATH}/slpp -d PI=3.141592654 -d AQSIS -c6 " << slppargs.c_str() << std::ends;
	aqslcompcommand << "${AQSIS_BASE_PATH}/aqslcomp " << aqslcompargs.c_str() << std::ends;
	hPipeRead = popen( slppcommand.str(), "r" );
	hPipeWrite = popen( aqslcompcommand.str(), "w" );
#else
	slppcommand << "slpp -d PI=3.141592654 -d AQSIS -c6 " << slppargs.c_str() << std::ends;
	aqslcompcommand << "aqslcomp " << aqslcompargs.c_str() << std::ends;
	hPipeRead = _popen( slppcommand.str(), "r" );
	hPipeWrite = _popen( aqslcompcommand.str(), "w" );
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
	std::string slppargs = "";
	std::string aqslcompargs = "";
	for ( i = 1; i < argc; i++ )
	{
		if ( strstr( argv[ i ], "-help" ) || 
			 strstr( argv[ i ], "-version" ) ||
			 strstr( argv[ i ], "-o" ) )
		{
			aqslcompargs.append( argv[i] );
			aqslcompargs.append(" ");
		}
		else
		{
			slppargs.append(argv[i]);
			slppargs.append(" ");
		}
	}
	std::cout << "slpp -- " << slppargs.c_str() << std::endl;
	std::cout << "aqslcomp -- " << aqslcompargs.c_str() << std::endl;
	compile_file( slppargs, aqslcompargs );

	return ( 0 );
}
