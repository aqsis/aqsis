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
		\brief Implements a Renderman Shading Language compiler that generates Aqsis bytecode 
		\author Paul C. Gregory (pgregory@aqsis.com)
		\author Timothy M. Shead (tshead@k-3d.com)
*/

#include	"aqsis.h"

#include	<iostream>
#include	<fstream>

#include	"libslparse.h"
#include	"vmoutput.h"
#include	"argparse.h"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include	"version.h"
#endif

using namespace Aqsis;


ArgParse::apstring	g_stroutname = "";
bool	g_help = 0;
bool	g_version = 0;


void version( std::ostream& Stream )
{
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
	Stream << "aqslcomp version " << VERSION_STR << std::endl;
#else
	Stream << "aqslcomp version " << VERSION << std::endl;
#endif
}


/** Process the sl file from stdin and produce an slx bytestream.
 */
int main( int argc, const char** argv )
{
	ArgParse ap;

	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options]" );
	ap.argFlag( "help", "\aprint this help and exit", &g_help );
	ap.argFlag( "version", "\aprint version information and exit", &g_version );
	ap.argString( "o", "=string \aspecify output filename", &g_stroutname );

	if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
	{
		std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
		exit( 1 );
	}

	if ( g_version )
	{
		version( std::cout );
		exit( 0 );
	}

	if ( g_help )
	{
		std::cout << ap.usagemsg();
		exit( 0 );
	}

	if ( Parse( std::cin, "stdin", std::cerr ) )
		OutputTree( GetParseTree(), g_stroutname );

	return 0;
}

