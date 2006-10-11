// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This program is free software; you can redistribute it and/or
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

#include "aqsis.h"
#include "exception.h"
#include "argparse.h"
#include "file.h"
#include "librib.h"
#include "librib2ri.h"
#include "logging.h"
#include "logging_streambufs.h"
#include "ri.h"
#include "version.h"
#include "bdec.h"
#include "parserstate.h"

#ifdef AQSIS_SYSTEM_WIN32
#include <io.h>
#endif // AQSIS_SYSTEM_WIN32
#include <fcntl.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <time.h>

#ifdef AQSIS_SYSTEM_WIN32
  #include <windows.h>
  #ifdef _DEBUG
    #include <crtdbg.h>
extern "C" __declspec(dllimport) void report_refcounts();
#endif // _DEBUG
#endif // !AQSIS_SYSTEM_WIN32

#if defined(AQSIS_SYSTEM_MACOSX)
#include "Carbon/Carbon.h"
#endif

// Forward declarations
void ProcessFile( FILE* file, std::string& name );

// Command-line arguments
ArgParse::apflag g_cl_pause;
ArgParse::apflag g_cl_nostandard = 0;
ArgParse::apflag g_cl_outstandard = 0;
ArgParse::apflag g_cl_help = 0;
ArgParse::apflag g_cl_version = 0;
ArgParse::apint g_cl_verbose = 1;
ArgParse::apstring g_cl_archive_path = "";
ArgParse::apflag g_cl_no_color = 0;
ArgParse::apstring g_cl_framesList = "";
ArgParse::apintvec g_cl_frames;
ArgParse::apint g_cl_indentation = 0;	// Default None
ArgParse::apint g_cl_indentlevel = 0;
ArgParse::apflag g_cl_binary = 0;
ArgParse::apint g_cl_compression = 0;	// Default None
ArgParse::apstring g_cl_output = "";

RtToken g_indentNone = "None";
RtToken g_indentSpace = "Space";
RtToken g_indentTab = "Tab";
RtToken g_typeBinary = "Binary";
RtToken g_compressionNone = "None";
RtToken g_compressionGzip = "Gzip";

#ifdef	AQSIS_SYSTEM_POSIX
ArgParse::apflag g_cl_syslog = 0;
#endif	// AQSIS_SYSTEM_POSIX

int main( int argc, const char** argv )
{
	ArgParse ap;
	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options] [RIB file...]" );
	ap.argFlag( "help", "\aPrint this help and exit", &g_cl_help );
	ap.alias( "help" , "h" );
	ap.argFlag( "version", "\aPrint version information and exit", &g_cl_version );
	ap.argFlag( "pause", "\aWait for a keypress on completion", &g_cl_pause );
	ap.argString( "output", "=string\aSet the output filename, default to <stdout>", &g_cl_output );
	ap.alias( "output", "o" );
	ap.argFlag( "nostandard", "\aDo not declare standard RenderMan parameters", &g_cl_nostandard );
	ap.argFlag( "outputstandard", "\aPrint the standard declarations to the resulting RIB", &g_cl_outstandard );
	ap.argInt( "verbose", "=integer\aSet log output level\n"
		   "\a0 = errors\n"
		   "\a1 = warnings (default)\n"
		   "\a2 = information\n"
		   "\a3 = debug", &g_cl_verbose );
	ap.alias( "verbose", "v" );
	ap.argInt( "indentation", "=integer\aSet output indentation type\n"
		   "\a0 = none (default)\n"
		   "\a1 = space\n"
		   "\a2 = tab", &g_cl_indentation );
	ap.alias( "indentation", "i" );
	ap.argInt( "indentlevel", "=integer\aSet the indetation amount", &g_cl_indentlevel);
	ap.alias( "indentlevel", "l" );
	ap.argInt( "compression", "=integer\aSet output compression type\n"
		   "\a0 = none (default)\n"
		   "\a1 = gzip", &g_cl_compression );
	ap.argFlag( "binary", "\aOutput a binary encoded RIB file", &g_cl_binary );
	ap.alias( "binary", "b" );
	ap.argInts( "frames", " f1 f2\aSpecify a starting/ending frame to render (inclusive).", &g_cl_frames, ArgParse::SEP_ARGV, 2);
	ap.argString( "frameslist", "=string\aSpecify a range of frames to render, ',' separated with '-' to indicate ranges.", &g_cl_framesList);
	ap.argFlag( "nocolor", "\aDisable colored output", &g_cl_no_color );
	ap.alias( "nocolor", "nc" );
#ifdef	AQSIS_SYSTEM_POSIX

	ap.argFlag( "syslog", "\aLog messages to syslog", &g_cl_syslog );
#endif	// AQSIS_SYSTEM_POSIX

	ap.argString( "archives", "=string\aOverride the default archive searchpath(s)", &g_cl_archive_path );
	ap.allowUnrecognizedOptions();

	//_crtBreakAlloc = 1305;

	if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
	{
		Aqsis::log() << ap.errmsg() << std::endl << ap.usagemsg();
		exit( 1 );
	}

	if ( g_cl_help )
	{
		std::cout << ap.usagemsg();
		exit( 0 );
	}

	if ( g_cl_version )
	{
		std::cout << "miqser version " << VERSION_STR
#ifdef _DEBUG
		<< " (debug build)"
#endif
		<< "\n"
		<< "compiled " << __DATE__ << " " << __TIME__ << "\n";
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
	Aqsis::log_level_t level = Aqsis::ERROR;
	if( g_cl_verbose > 0 )
		level = Aqsis::WARNING;
	if( g_cl_verbose > 1 )
		level = Aqsis::INFO;
	if( g_cl_verbose > 2 )
		level = Aqsis::DEBUG;
	std::auto_ptr<std::streambuf> filter_level( new Aqsis::filter_by_level_buf(level, Aqsis::log()) );
#ifdef	AQSIS_SYSTEM_POSIX

	if( g_cl_syslog )
		std::auto_ptr<std::streambuf> use_syslog( new Aqsis::syslog_buf(Aqsis::log()) );
#endif	// AQSIS_SYSTEM_POSIX

	// Setup the indentation if specified
	if(g_cl_indentation > 0)
	{
		RtToken itype[1];
		if(g_cl_indentation == 1)
			itype[0] = g_indentSpace;
		else if(g_cl_indentation == 2)
			itype[0] = g_indentTab;
		else 
			itype[0] = g_indentNone;
		RiOption("RI2RIB_Indentation", "Type", &itype, RI_NULL);
		// Output indentation level if specified
		if(g_cl_indentlevel > 0)
		{
			RtInt isize[1];
			isize[0] = g_cl_indentlevel;
			RiOption("RI2RIB_Indentation", "Size", &isize, RI_NULL);
		}
	}

	// Set the output type to binary if specified.
	if(g_cl_binary)
	{
		RtToken itype[1];
		itype[0] = g_typeBinary;
		RiOption("RI2RIB_Output", "Type", &itype, RI_NULL);
	}

	// Setup the compression if specified
	if(g_cl_compression > 0)
	{
		RtToken itype[1];
		if(g_cl_compression == 1)
			itype[0] = g_compressionGzip;
		else
			itype[0] = g_compressionNone;
		RiOption("RI2RIB_Output", "Compression", &itype, RI_NULL);
	}

	if ( ap.leftovers().size() == 0 )     // If no files specified, take input from stdin.
	{
		std::string name("stdin");
		ProcessFile( stdin, name );
	}
	else
	{
		for ( ArgParse::apstringvec::const_iterator e = ap.leftovers().begin(); e != ap.leftovers().end(); e++ )
		{
			FILE *file = fopen( e->c_str(), "rb" );
			if ( file != NULL )
			{
				std::string name(*e);
				ProcessFile( file, name );
				fclose( file );
			}
			else
			{
				std::cout << "Warning: Cannot open file \"" << *e << "\"" << std::endl;
			}
		}
	}

	if(g_cl_pause)
	{
		std::cout << "Press any key..." << std::ends;
		std::cin.ignore(std::cin.rdbuf()->in_avail() + 1);
	}

	return ( 0 );
}

void ProcessFile( FILE* file, std::string&  name )
{
	librib::RendermanInterface * engine = librib2ri::CreateRIBEngine();

	try
	{
		if(g_cl_output.compare("")!=0)
		{
			char* outputName = new char[g_cl_output.size()+1];
			strcpy(outputName, g_cl_output.c_str());
			outputName[g_cl_output.size()] = '\0';
			RiBegin(outputName);
		}
		else
			RiBegin(RI_NULL);

		if ( !g_cl_nostandard )
		{
			if( !g_cl_outstandard )
				librib::StandardDeclarations( NULL );
			else
				librib::StandardDeclarations( engine );
		}


		const char* popt[1];
		if(!g_cl_archive_path.empty())
		{
			popt[0] = g_cl_archive_path.c_str();
			RiOption( "searchpath", "archive", &popt, RI_NULL );
		}

		librib::ClearFrames();
		// Pass in specified frame lists.
		if(g_cl_frames.size() == 2)
		{
			std::stringstream strframes;
			strframes << g_cl_frames[0] << "-" << g_cl_frames[1] << std::ends;
			librib::AppendFrames(strframes.str().c_str());
		}
		if(!g_cl_framesList.empty())
			librib::AppendFrames(g_cl_framesList.c_str());

		librib::Parse( file, name, *engine, Aqsis::log(), NULL );

		RiEnd();

		if ( !g_cl_nostandard )
			librib::CleanupDeclarations( *engine );
	}
	catch(Aqsis::XqException& x)
	{
		Aqsis::log() << Aqsis::error << x.strReason().c_str() << std::endl;
	}

	librib2ri::DestroyRIBEngine( engine );
}
