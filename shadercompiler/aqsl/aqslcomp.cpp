// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
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
		\author Paul C. Gregory (pgregory@aqsis.org)
		\author Timothy M. Shead (tshead@k-3d.com)
*/

#include	"aqsis.h"
#include	"logging.h"
#include	"logging_streambufs.h"

#include	<iostream>
#include	<fstream>
#include	<sstream>
#include	<stdio.h>

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

#include	"version.h"

using namespace Aqsis;

extern "C" void PreProcess(int argc, char** argv);

ArgParse::apstring	g_stroutname = "";
bool	g_help = 0;
bool	g_version = 0;
ArgParse::apstringvec g_defines; // Filled in with strings to pass to the preprocessor
ArgParse::apstringvec g_includes; // Filled in with strings to pass to the preprocessor
ArgParse::apstringvec g_undefines; // Filled in with strings to pass to the preprocessor

bool g_cl_no_color = false;
bool g_cl_syslog = false;

void version( std::ostream& Stream )
{
	Stream << "aqsl version " << VERSION_STR_PRINT << std::endl << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
}


char* g_slppDefArgs[] =
    {
        "slpp",
        "-d",
        "PI=3.141592654",
        "-d",
        "AQSIS",
        "-c6",
    };
int g_cslppDefArgs = sizeof( g_slppDefArgs ) / sizeof( g_slppDefArgs[0] );


/** Process the sl file from stdin and produce an slx bytestream.
 */
int main( int argc, const char** argv )
{
	ArgParse ap;
	CqCodeGenVM codegen; // Should be a pointer determined by what we want to generate
	bool error = false; ///! Couldn't compile shader

	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options] <filename>" );
	ap.argString( "o", " %s \aspecify output filename", &g_stroutname );
	ap.argStrings( "i", "%s \aSet path for #include files.", &g_includes );
	ap.argStrings( "I", "%s \aSet path for #include files.", &g_includes );
	ap.argStrings( "D", "Sym[=value] \adefine symbol <string> to have value <value> (default 1).", &g_defines );
	ap.argStrings( "U", "Sym \aUndefine an initial symbol.", &g_undefines );
	ap.argFlag( "help", "\aprint this help and exit", &g_help );
	ap.argFlag( "version", "\aprint version information and exit", &g_version );

	if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
	{
		Aqsis::log() << ap.errmsg() << std::endl << ap.usagemsg();
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

#ifdef	AQSIS_SYSTEM_WIN32
	std::auto_ptr<std::streambuf> ansi( new Aqsis::ansi_buf(Aqsis::log()) );
#endif

	std::auto_ptr<std::streambuf> reset_level( new Aqsis::reset_level_buf(Aqsis::log()) );
	std::auto_ptr<std::streambuf> show_timestamps( new Aqsis::timestamp_buf(Aqsis::log()) );
	std::auto_ptr<std::streambuf> fold_duplicates( new Aqsis::fold_duplicates_buf(Aqsis::log()) );
	std::auto_ptr<std::streambuf> color_level;
	if(!g_cl_no_color)
	{
		std::auto_ptr<std::streambuf> temp_color_level( new Aqsis::color_level_buf(Aqsis::log()) );
		color_level = temp_color_level;
	}
	std::auto_ptr<std::streambuf> show_level( new Aqsis::show_level_buf(Aqsis::log()) );
	std::auto_ptr<std::streambuf> filter_level( new Aqsis::filter_by_level_buf(Aqsis::DEBUG, Aqsis::log()) );
#ifdef	AQSIS_SYSTEM_POSIX

	if( g_cl_syslog )
		std::auto_ptr<std::streambuf> use_syslog( new Aqsis::syslog_buf(Aqsis::log()) );
#endif	// AQSIS_SYSTEM_POSIX

	// Pass the shader file through the slpp preprocessor first to generate a temporary file.
	if ( ap.leftovers().size() == 0 )     // If no files specified, take input from stdin.
	{
		//if ( Parse( std::cin, "stdin", Aqsis::log() ) )
		//	codegen.OutputTree( GetParseTree(), g_stroutname );
		std::cout << ap.usagemsg();
		exit( 0 );
	}
	else
	{
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
					// Build the arguments array for slpp.
					std::vector<char*>	slppArgs;
					for ( int defArg = 0; defArg != g_cslppDefArgs; defArg++ )
					{
						char* Arg = new char[strlen(g_slppDefArgs[ defArg ]) + 1];
						sprintf( Arg, g_slppDefArgs[ defArg ] );
						slppArgs.push_back(Arg);
					}

					// Append the -d arguments passed in to forward them to the preprocessor.
					for ( ArgParse::apstringvec::const_iterator define = g_defines.begin(); define != g_defines.end(); define++ )
					{
						char* Arg = new char[strlen("-d") + 1];
						strcpy( Arg, "-d" );
						slppArgs.push_back(Arg);
						Arg = new char[define->size() + 1];
						strcpy( Arg, define->c_str() );
						slppArgs.push_back(Arg);
					}

					// Append the -i arguments passed in to forward them to the preprocessor.
					for ( ArgParse::apstringvec::const_iterator include = g_includes.begin(); include != g_includes.end(); include++ )
					{
						char* Arg = new char[strlen("-i") + 1];
						strcpy( Arg, "-i" );
						slppArgs.push_back(Arg);
						Arg = new char[include->size() + 1];
						strcpy( Arg, include->c_str() );
						slppArgs.push_back(Arg);
					}

					// Append the -u arguments passed in to forward them to the preprocessor.
					for ( ArgParse::apstringvec::const_iterator undefine = g_undefines.begin(); undefine != g_undefines.end(); undefine++ )
					{
						char* Arg = new char[strlen("-u") + 1];
						strcpy( Arg, "-u" );
						slppArgs.push_back(Arg);
						Arg = new char[undefine->size() + 1];
						strcpy( Arg, undefine->c_str() );
						slppArgs.push_back(Arg);
					}

					// Set the output filename.
					char* Arg = new char[strlen("-o") + 1];
					strcpy( Arg, "-o" );
					slppArgs.push_back(Arg);
					Arg = new char[strlen(tempname) + 1];
					strcpy( Arg, tempname );
					slppArgs.push_back(Arg);

					// Set the input filename.
					char* fileArg = new char[e->size() + 1];
					sprintf( fileArg, e->c_str() );
					slppArgs.push_back(fileArg);

					PreProcess(slppArgs.size(),&slppArgs[0]);

					std::ifstream ppfile( tempname );
					if ( Parse( ppfile, e->c_str(), Aqsis::log() ) )
						codegen.OutputTree( GetParseTree(), g_stroutname );
					else
						error = true;

					// Delete the temporary file.
					ppfile.close();
					remove
						(tempname);
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

	return error ? -1 : 0;
}

