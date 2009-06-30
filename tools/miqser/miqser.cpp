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

/** \file
		\brief Tool 'miqser' (mixer) for processing RIB files.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/


//------------------------------------------------------------------------------
#include <aqsis/aqsis.h>

#ifdef AQSIS_SYSTEM_WIN32
#	include <windows.h>
#	include <io.h>
#	ifdef _DEBUG
#		include <crtdbg.h>
		extern "C" __declspec(dllimport) void report_refcounts();
#	endif
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <time.h>

#include <aqsis/util/argparse.h>
#include <aqsis/util/exception.h>
#include <aqsis/util/file.h>
#include <aqsis/ribparser.h>
#include <aqsis/util/logging.h>
#include <aqsis/util/logging_streambufs.h>
#include <aqsis/ri/ri.h>
#include <aqsis/riutil/ri_convenience.h>
#include "ribrequesthandler.h"
#include <aqsis/version.h>


//------------------------------------------------------------------------------
// Forward declarations
void setupOutputFormat();
void processFiles(const ArgParse::apstringvec& fileNames);
void parseAndFormat(std::istream& ribStream, const std::string& name);
void parseRibStream(std::istream& ribStream, const std::string& name);

// Command-line arguments
ArgParse::apflag g_cl_pause;
ArgParse::apflag g_cl_outstandard = 0;
ArgParse::apflag g_cl_help = 0;
ArgParse::apflag g_cl_version = 0;
ArgParse::apint g_cl_verbose = 1;
ArgParse::apstring g_cl_archive_path = "";
ArgParse::apflag g_cl_no_color = 0;
ArgParse::apint g_cl_indentation = 0;	// Default None
ArgParse::apint g_cl_indentlevel = 0;
ArgParse::apflag g_cl_binary = 0;
ArgParse::apint g_cl_compression = 0;	// Default None
ArgParse::apstring g_cl_output = "";

RtToken g_indentNone      = tokenCast("None");
RtToken g_indentSpace     = tokenCast("Space");
RtToken g_indentTab       = tokenCast("Tab");
RtToken g_typeBinary      = tokenCast("Binary");
RtToken g_compressionNone = tokenCast("None");
RtToken g_compressionGzip = tokenCast("Gzip");

#ifdef	AQSIS_SYSTEM_POSIX
ArgParse::apflag g_cl_syslog = 0;
#endif	// AQSIS_SYSTEM_POSIX


//------------------------------------------------------------------------------
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
		std::cout << "miqser version " << AQSIS_VERSION_STR_FULL
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

	setupOutputFormat();
	processFiles(ap.leftovers());

	if(g_cl_pause)
	{
		std::cout << "Press any key..." << std::ends;
		std::cin.ignore(std::cin.rdbuf()->in_avail() + 1);
	}

	return ( 0 );
}


//------------------------------------------------------------------------------
/** \brief Set up the output formatting, compression etc for libri2rib.
 */
void setupOutputFormat()
{
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
		RiOption(tokenCast("RI2RIB_Indentation"), "Type", &itype, RI_NULL);
		// Output indentation level if specified
		if(g_cl_indentlevel > 0)
		{
			RtInt isize[1];
			isize[0] = g_cl_indentlevel;
			RiOption(tokenCast("RI2RIB_Indentation"), "Size", &isize, RI_NULL);
		}
	}

	// Set the output type to binary if specified.
	if(g_cl_binary)
	{
		RtToken itype[1];
		itype[0] = g_typeBinary;
		RiOption(tokenCast("RI2RIB_Output"), "Type", &itype, RI_NULL);
	}

	// Setup the compression if specified
	if(g_cl_compression > 0)
	{
		RtToken itype[1];
		if(g_cl_compression == 1)
			itype[0] = g_compressionGzip;
		else
			itype[0] = g_compressionNone;
		RiOption(tokenCast("RI2RIB_Output"), "Compression", &itype, RI_NULL);
	}
}


//------------------------------------------------------------------------------
/** \brief Process all RIB files.
 */
void processFiles(const ArgParse::apstringvec& fileNames)
{
	if ( fileNames.empty() )
	{
		// If no files specified, take input from stdin.
		parseAndFormat(std::cin, "stdin");
	}
	else
	{
		for ( ArgParse::apstringvec::const_iterator e = fileNames.begin(); e != fileNames.end(); e++ )
		{
			std::ifstream file(e->c_str(), std::ios::binary);
			if ( file != NULL )
				parseAndFormat(file, *e);
			else
				std::cout << "Warning: Cannot open file \"" << *e << "\"\n";
		}
	}
}


//------------------------------------------------------------------------------
/** \brief Parse the RIB file and format the result with libri2rib
 */
void parseAndFormat(std::istream& ribStream, const std::string& name)
{
	try
	{
		if(g_cl_output != "")
			RiBegin(tokenCast(g_cl_output.c_str()));
		else
			RiBegin(RI_NULL);

		const char* popt[1];
		if(!g_cl_archive_path.empty())
		{
			popt[0] = g_cl_archive_path.c_str();
			RiOption( tokenCast("searchpath"), "archive", &popt, RI_NULL );
		}

		parseRibStream(ribStream, name);

		RiEnd();
	}
	catch(Aqsis::XqException& x)
	{
		Aqsis::log() << Aqsis::error << x.what() << std::endl;
	}
}


/** Parse an open RIB stream, sending all the commands to the current RI context.
 */
void parseRibStream(std::istream& ribStream, const std::string& name)
{
	boost::shared_ptr<Aqsis::IqRibParser> ribParser =
		Aqsis::IqRibParser::create( boost::shared_ptr<Aqsis::IqRibRequestHandler>(
				new Aqsis::CqRibRequestHandler()) );
	ribParser->pushInput(ribStream, name,
			Aqsis::CqArchiveCallbackAdaptor(RiArchiveRecord));
	bool parsing = true;
	while(parsing)
	{
		try
		{
			parsing = ribParser->parseNextRequest();
		}
		catch(Aqsis::XqParseError& e)
		{
			Aqsis::log() << Aqsis::error << e.what() << "\n";
		}
	}
	ribParser->popInput();
}
