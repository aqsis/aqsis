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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

#include <boost/shared_ptr.hpp>

#include <aqsis/util/argparse.h>
#include <aqsis/util/exception.h>
#include <aqsis/util/logging.h>
#include <aqsis/util/logging_streambufs.h>
#include <aqsis/version.h>

#include <aqsis/riutil/ricxxutil.h>
#include <aqsis/riutil/ribwriter.h>


// Command-line arguments
ArgParse::apflag g_cl_pause = false;
ArgParse::apflag g_cl_outstandard = false;
ArgParse::apflag g_cl_help = false;
ArgParse::apflag g_cl_version = false;
ArgParse::apint g_cl_verbose = 1;
ArgParse::apflag g_cl_no_color = false;
ArgParse::apstring g_cl_frameList = "";
ArgParse::apintvec g_cl_frames;
ArgParse::apstring g_cl_indentation = "space";
ArgParse::apint g_cl_compression = 0;	// Default None
ArgParse::apstring g_cl_output = "";
Aqsis::RibWriterOptions g_writerOpts;
#ifdef AQSIS_SYSTEM_POSIX
ArgParse::apflag g_cl_syslog = false;
#endif


// Get list of frames, as a string
std::string getFrameList()
{
	std::string frameList;
	if(g_cl_frames.size() == 2)
	{
		std::ostringstream fmt;
		fmt << g_cl_frames[0] << "-" << g_cl_frames[1];
		frameList = fmt.str();
	}
	if(!g_cl_frameList.empty())
	{
		if(!frameList.empty())
			frameList += ',';
		frameList += g_cl_frameList;
	}
	return frameList;
}


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
	ap.argString( "indentation", "=string\aSet output indentation type\n"
	              "\aspace or 1 (default)\n"
	              "\atab or 2", &g_cl_indentation );
	ap.alias( "indentation", "i" );
	ap.argInt( "indentlevel", "=integer\aSet the indetation amount "
			   "(default %default)", &g_writerOpts.indentStep);
	ap.alias( "indentlevel", "l" );
	ap.argInt( "compression", "=integer\aSet output compression type\n"
	           "\a0 = none (default)\n"
	           "\a1 = gzip", &g_cl_compression );
	ap.argFlag( "binary", "\aOutput a binary encoded RIB file", &g_writerOpts.useBinary );
	ap.alias( "binary", "b" );
	ap.argInts( "frames", " f1 f2\aSpecify a starting/ending frame to render (inclusive).", &g_cl_frames, ArgParse::SEP_ARGV, 2);
	ap.argString( "framelist", "=string\aSpecify a range of frames to render, ',' separated with '-' to indicate ranges.", &g_cl_frameList);
	ap.argFlag( "nocolor", "\aDisable colored output", &g_cl_no_color );
	ap.alias( "nocolor", "nc" );
#ifdef AQSIS_SYSTEM_POSIX
	ap.argFlag( "syslog", "\aLog messages to syslog", &g_cl_syslog );
#endif
	ap.argString( "archives", "=string\aOverride the initial archive searchpath(s) (default \"%default\")", &g_writerOpts.archivePath );
	ap.argFlag( "readarchives", "\aInterpolate all ReadArchive calls into the output", &g_writerOpts.interpolateArchives );
	ap.alias("readarchives", "ra");
	ap.allowUnrecognizedOptions();

	if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
	{
		Aqsis::log() << ap.errmsg() << std::endl << ap.usagemsg();
		return EXIT_FAILURE;
	}

	if ( g_cl_help )
	{
		std::cerr << ap.usagemsg();
		return EXIT_SUCCESS;
	}

	if ( g_cl_version )
	{
		std::cerr << "miqser version " << AQSIS_VERSION_STR_FULL
#ifdef _DEBUG
		<< " (debug build)"
#endif
		<< "\n"
		<< "compiled " << __DATE__ << " " << __TIME__ << "\n";
		exit( 0 );
	}

#ifdef AQSIS_SYSTEM_WIN32
	Aqsis::ansi_buf ansi(Aqsis::log());
#endif
	Aqsis::reset_level_buf reset_level(Aqsis::log());
//	Aqsis::fold_duplicates_buf fold_duplicates(Aqsis::log());
	boost::shared_ptr<std::streambuf> color_level;
	if(!g_cl_no_color)
		color_level.reset(new Aqsis::color_level_buf(Aqsis::log()));
	Aqsis::show_level_buf show_level(Aqsis::log());
	Aqsis::log_level_t level = Aqsis::ERROR;
	if( g_cl_verbose > 0 )
		level = Aqsis::WARNING;
	if( g_cl_verbose > 1 )
		level = Aqsis::INFO;
	if( g_cl_verbose > 2 )
		level = Aqsis::DEBUG;
	Aqsis::filter_by_level_buf filter_level(level, Aqsis::log());
#ifdef AQSIS_SYSTEM_POSIX
	boost::shared_ptr<std::streambuf> use_syslog;
	if( g_cl_syslog )
		use_syslog.reset(new Aqsis::syslog_buf(Aqsis::log()));
#endif

	namespace Ri = Aqsis::Ri;

	// get indentation
	g_writerOpts.indentStep = std::max(0, g_writerOpts.indentStep);
	// Accept 0/1/2 here for compatibility with <= v1.6
	if(g_cl_indentation == "space" || g_cl_indentation == "1")
		g_writerOpts.indentChar = ' ';
	else if(g_cl_indentation == "tab" || g_cl_indentation == "2")
		g_writerOpts.indentChar = '\t';
	else if(g_cl_indentation == "0")
		g_writerOpts.indentStep = 0;
	else
		Aqsis::log() << Aqsis::warning << "unknown indent character";
	g_writerOpts.useGzip = g_cl_compression;

	// Get output stream
	std::ostream* outStream = &std::cout;
	std::ofstream outFile;
	if(!g_cl_output.empty())
	{
		outFile.open(g_cl_output.c_str(), std::ios::binary);
		if(!outFile)
			return EXIT_FAILURE;
		outStream = &outFile;
	}

	// Open writer
	boost::shared_ptr<Ri::RendererServices> writer(
		Aqsis::createRibWriter(*outStream, g_writerOpts));
	// Add frame filter if desired
	std::string frameList = getFrameList();
	if(!frameList.empty())
		writer->addFilter("framedrop", Aqsis::ParamListBuilder()
						  ("frames", frameList.c_str()));
	// Add interface validation, with relaxed outer scope handling so that
	// miqser won't reject RIB fragments which don't start in the outer scope.
	writer->addFilter("validate", Aqsis::ParamListBuilder()
					  ("relaxed_outer_scope", 1));

	// Parse files
	const ArgParse::apstringvec& fileNames = ap.leftovers();
	if(fileNames.empty())
	{
		// If no files specified, take input from stdin.
		writer->parseRib(std::cin, "stdin", writer->firstFilter());
	}
	else
	{
		for(ArgParse::apstringvec::const_iterator fileName = fileNames.begin();
			fileName != fileNames.end(); ++fileName)
		{
			std::ifstream file(fileName->c_str(), std::ios::binary);
			if(file)
				writer->parseRib(file, fileName->c_str(),
								 writer->firstFilter());
			else
				std::cerr << "Warning: Cannot open file \""
						  << *fileName << "\"\n";
		}
	}

	if(g_cl_pause)
	{
		std::cerr << "Press any key..." << std::ends;
		std::cin.ignore(std::cin.rdbuf()->in_avail() + 1);
	}
	return EXIT_SUCCESS;
}
