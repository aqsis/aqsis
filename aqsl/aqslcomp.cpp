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
#include	<sstream>

#ifdef	AQSIS_SYSTEM_WIN32
#include	"io.h"
#else
#include	"unistd.h"
#endif //AQSIS_SYSTEM_WIN32

#include	"libslparse.h"
#include	"icodegen.h"
#include	"codegenvm.h"
#include	"vmoutput.h"
#include	"argparse.h"

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include	"version.h"
#endif

using namespace Aqsis;


ArgParse::apstring	g_stroutname = "";
bool	g_help = 0;
bool	g_version = 0;
ArgParse::apstringvec g_defines; // Filled in with strings to pass to the preprocessor
ArgParse::apstringvec g_includes; // Filled in with strings to pass to the preprocessor
ArgParse::apstringvec g_undefines; // Filled in with strings to pass to the preprocessor


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
	CqCodeGenVM codegen; // Should be a pointer determined by what we want to generate

	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options]" );
	ap.argFlag( "help", "\aprint this help and exit", &g_help );
	ap.argFlag( "version", "\aprint version information and exit", &g_version );
	ap.argString( "o", " string \aspecify output filename", &g_stroutname );
	ap.argStrings( "d", " string[=value] \adefine symbol <string> to have value <value> (default 1).", &g_defines );
	ap.argStrings( "i", " string \aSet path for #include files.", &g_includes );
	ap.argStrings( "u", " string \aUndefine an initial symbol.", &g_undefines );

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

	// Pass the shader file through the slpp preprocessor first to generate a temporary file.
	if ( ap.leftovers().size() == 0 )     // If no files specified, take input from stdin.
	{
		//if ( Parse( std::cin, "stdin", std::cerr ) )
		//	codegen.OutputTree( GetParseTree(), g_stroutname );
		std::cout << ap.usagemsg();
		exit( 0 );
	}
	else
	{
		std::stringstream strCommand;
		strCommand << "slpp -d PI=3.141592654 -d AQSIS -c6 ";
		// Append the -d arguments passed in to forward them to the preprocessor.
		for ( ArgParse::apstringvec::const_iterator define = g_defines.begin(); define != g_defines.end(); define++ )
			strCommand << "-d " << define->c_str() << " ";

		// Append the -i arguments passed in to forward them to the preprocessor.
		for ( ArgParse::apstringvec::const_iterator include = g_includes.begin(); include != g_includes.end(); include++ )
			strCommand << "-i " << include->c_str() << " ";

		// Append the -u arguments passed in to forward them to the preprocessor.
		for ( ArgParse::apstringvec::const_iterator undefine = g_undefines.begin(); undefine != g_undefines.end(); undefine++ )
			strCommand << "-u " << undefine->c_str() << " ";

		const char* _template = "slppXXXXXX";
		char ifile[11];
		for ( ArgParse::apstringvec::const_iterator e = ap.leftovers().begin(); e != ap.leftovers().end(); e++ )
		{
			FILE *file = fopen( e->c_str(), "rb" );
			if ( file != NULL )
			{
				fclose(file);
				strcpy( ifile, _template );
				char* tempname;
				#ifdef	AQSIS_SYSTEM_WIN32
				tempname = _mktemp( ifile );
				#else
				tempname = mktemp( ifile );
				#endif //AQSIS_SYSTEM_WIN32
				if( NULL != tempname )
				{
					std::stringstream strThisCommand;
					strThisCommand << strCommand.str();
					// Set the output filename.
					strThisCommand << "-o " << tempname << " ";
					strThisCommand << e->c_str() << std::ends;
					system( strThisCommand.str().c_str() );

					std::ifstream ppfile( tempname );
					if ( Parse( ppfile, e->c_str(), std::cerr ) )
						codegen.OutputTree( GetParseTree(), g_stroutname );

					// Delete the temporary file.
					ppfile.close();
					remove(tempname);
				}
				else
				{
					std::cout << "Could not create temporary file for preprocessing." << std::endl;
					exit( -1 );
				}
			}
			else
			{
				std::cout << "Warning: Cannot open file \"" << *e << "\"" << std::endl;
			}
		}
	}

	return 0;
}

