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
#include "exception.h"

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

#if defined(AQSIS_SYSTEM_WIN32) && defined(_DEBUG)

#define StartMemoryDebugging() \
    std::ostringstream __buffer; \
    __buffer << 0; \
    _CrtMemState __initialState; \
    _CrtMemCheckpoint(&__initialState);

#define StopMemoryDebugging() \
    _CrtMemDumpAllObjectsSince(&__initialState); \
    MEMORY_BASIC_INFORMATION mbi; \
    DWORD dwMemUsed = 0; \
    PVOID pvAddress = 0; \
    SYSTEM_INFO SystemInfo; \
    memset( &SystemInfo, 0, sizeof( SYSTEM_INFO ) ); \
    GetSystemInfo( &SystemInfo ); \
    memset( &mbi, 0, sizeof( MEMORY_BASIC_INFORMATION ) ); \
    while ( VirtualQuery( pvAddress, &mbi, sizeof( MEMORY_BASIC_INFORMATION ) ) == sizeof( MEMORY_BASIC_INFORMATION ) ) \
    { \
        if ( mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE ) \
        dwMemUsed += mbi.RegionSize; \
        pvAddress = ( ( BYTE* ) mbi.BaseAddress ) + mbi.RegionSize; \
    } \
    std::cout << "Peak Memory Used " << dwMemUsed << std::endl; \
    report_refcounts();

#else

#define StartMemoryDebugging()
#define StopMemoryDebugging()

#endif

#if defined(AQSIS_SYSTEM_MACOSX)
#include "Carbon/Carbon.h"
#endif

#if !defined(AQSIS_SYSTEM_WIN32)
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

// Forward declarations
void RenderFile( FILE* file, std::string& name );

// Command-line arguments
ArgParse::apflag g_cl_pause;
// Set verbose stats if in debug mode
#ifdef	_DEBUG
ArgParse::apint g_cl_endofframe = 3;
#else
ArgParse::apint g_cl_endofframe = -1;
#endif
ArgParse::apflag g_cl_nostandard = 0;
ArgParse::apflag g_cl_help = 0;
ArgParse::apint g_cl_priority = 1;
ArgParse::apflag g_cl_version = 0;
ArgParse::apflag g_cl_fb = 0;
ArgParse::apflag g_cl_progress = 0;
ArgParse::apflag g_cl_Progress = 0;
ArgParse::apflag g_cl_no_color = 0;
ArgParse::apflag g_cl_beep = 0;
ArgParse::apint g_cl_verbose = 1;
ArgParse::apflag g_cl_echoapi = 0;
ArgParse::apfloatvec g_cl_cropWindow;
ArgParse::apstring g_cl_rc_path = "";
ArgParse::apstring g_cl_shader_path = "";
ArgParse::apstring g_cl_archive_path = "";
ArgParse::apstring g_cl_texture_path = "";
ArgParse::apstring g_cl_display_path = "";
ArgParse::apstring g_cl_procedural_path = "";
ArgParse::apstring g_cl_plugin_path = "";
ArgParse::apstring g_cl_type = "";
ArgParse::apstring g_cl_addtype = "";
ArgParse::apstring g_cl_mode = "rgba";
ArgParse::apstring g_cl_strprogress = "Frame (%f) %p%% complete [ %s secs / %S left ]";
ArgParse::apstring g_cl_framesList = "";
ArgParse::apintvec g_cl_frames;
ArgParse::apintvec g_cl_res;
ArgParse::apstringvec g_cl_options;

#if ENABLE_MPDUMP
ArgParse::apflag g_cl_mpdump = 0;
#endif

#ifdef	AQSIS_SYSTEM_POSIX
ArgParse::apflag g_cl_syslog = 0;
#endif	// AQSIS_SYSTEM_POSIX

/// Function to print the progress of the render.  Used as the callback function to a RiProgressHandler call.
RtVoid PrintProgress( RtFloat percent, RtInt FrameNo )
{
	if ( ( g_cl_progress == 0 ) && ( g_cl_Progress == 0 ) )
		return ;

	// If g_Progress is set, 100% have to be reported. In all other cases the 100% are not displayed
	if ( percent >= 100 && !g_cl_Progress )
	{
		std::cout << "                                                                              \r" << std::flush;
		return ;
	}

	static time_t tick = 0;
	time_t now;

	if ( tick == 0 )
		time( &tick );

	time( &now );

	// Calculate the various values for putting in the string.
#ifdef AQSIS_SYSTEM_MACOSX

	TqFloat total_secs = ( RtFloat ) 100.0f * ( ( RtFloat ) ( now - tick ) / ( float ) CLOCKS_PER_SEC );
#elif	AQSIS_SYSTEM_POSIX

	TqFloat total_secs = ( RtFloat ) 1000000.0f * ( ( RtFloat ) ( now - tick ) / ( float ) CLOCKS_PER_SEC );
#else

	TqFloat total_secs = ( RtFloat ) 1000.0f * ( ( RtFloat ) ( now - tick ) / ( float ) CLOCKS_PER_SEC );
#endif

	TqFloat total_mins = total_secs / 60.0f;
	TqFloat total_hrs = total_mins / 60.0f;
	TqFloat sub_secs = total_secs - ( ( TqInt ) total_mins * 60.0f );
	TqFloat sub_mins = total_mins - ( ( TqInt ) total_hrs * 60.0f );

	TqFloat total_secsleft = ( ( ( RtFloat ) 100 / percent ) * total_secs ) - total_secs;
	TqFloat total_minsleft = total_secsleft / 60.0f;
	TqFloat total_hrsleft = total_minsleft / 60.0f;
	TqFloat sub_secsleft = total_secsleft - ( ( TqInt ) total_minsleft * 60.0f );
	TqFloat sub_minsleft = total_minsleft - ( ( TqInt ) total_hrsleft * 60.0f );

	// Now print the line with substitution.
	std::string::size_type ipos = 0;

	std::string strProgress;

	static int last_percent = 0;  // So we can skip instead of printing the same twice
	if ( g_cl_Progress )  // Override the outputformat
	{
		strProgress = "R90000%p%%";
		percent = static_cast<int>( percent );
		if ( last_percent == percent )
			return;
		else
			last_percent = static_cast<int>(percent);
	}
	else			// Use the default style
	{
		strProgress = g_cl_strprogress;
	}

	std::ostringstream strOutput;
	strOutput.setf( std::ios::fixed )
		;
	while ( 1 )
	{
		std::string::size_type itag;
		itag = strProgress.find( '%', ipos );
		if ( itag == std::string::npos )
		{
			strOutput << strProgress.substr( ipos ).c_str();
			break;
		}
		else
		{
			if ( ipos != itag )
				strOutput << strProgress.substr( ipos, itag - ipos ).c_str();

			switch ( strProgress[ itag + 1 ] )
			{
					case 'p':
					strOutput << std::setw( 6 ) << std::setfill( ' ' ) << std::setprecision( 2 ) << percent;
					break;

					case 's':
					strOutput << std::setprecision( 0 ) << ( TqInt ) total_secs;
					break;

					case 'S':
					strOutput << std::setprecision( 0 ) << ( TqInt ) total_secsleft;
					break;

					case 'm':
					strOutput << std::setprecision( 0 ) << ( TqInt ) total_mins;
					break;

					case 'M':
					strOutput << std::setprecision( 0 ) << ( TqInt ) total_minsleft;
					break;

					case 'h':
					strOutput << std::setprecision( 0 ) << ( TqInt ) total_hrs;
					break;

					case 'H':
					strOutput << std::setprecision( 0 ) << ( TqInt ) total_hrsleft;
					break;

					case 't':
					strOutput << std::setprecision( 0 ) << ( TqInt ) total_hrs << ":" << ( TqInt ) sub_mins << ":" << ( TqInt ) sub_secs;
					break;

					case 'T':
					strOutput << std::setprecision( 0 ) << ( TqInt ) total_hrsleft << ":" << ( TqInt ) sub_minsleft << ":" << ( TqInt ) sub_secsleft;
					break;

					case 'f':
					strOutput << std::setprecision( 0 ) << ( TqInt ) FrameNo;
					break;

					case '%':
					strOutput << '%';
					break;
			}
			ipos = itag + 2;
		}

		if ( ipos >= strProgress.size() )
			break;
	}
	// Pad to the end of the line.
	while ( strOutput.str().size() < 79 )
		strOutput << " ";

	std::cout << strOutput.str();

	if ( g_cl_Progress )
		std::cout << "\n";
	else
		std::cout << "\r";

	std:: cout << std::flush;
}


/** Function to setup specific options needed after options are complete but before the world is created.
	Used as the callback function to a RiPreWorldFunction call.
 */
#ifdef	AQSIS_SYSTEM_BEOS
RtVoid PreWorld( ... )
#else
RtVoid PreWorld()
#endif
{
	if ( g_cl_fb )
	{
		RiDisplay( "aqsis", "framebuffer", "rgb", NULL );
	}
	else if ( !g_cl_type.empty() )
	{
		RiDisplay( "aqsis", const_cast<char*>(g_cl_type.c_str()), const_cast<char*>(g_cl_mode.c_str()), NULL );
	}
	else if ( !g_cl_addtype.empty() )
	{
		RiDisplay( "+aqsis", const_cast<char*>(g_cl_addtype.c_str()), const_cast<char*>(g_cl_mode.c_str()), NULL );
	}

	// Pass the statistics option onto Aqsis.
	if ( g_cl_endofframe >= 0 )
	{
		RiOption( "statistics", "endofframe", &g_cl_endofframe, RI_NULL );
	}

	// Pass the crop window onto Aqsis.
	if( g_cl_cropWindow.size() == 4 )
	{
		RiCropWindow(g_cl_cropWindow[0], g_cl_cropWindow[1], g_cl_cropWindow[2], g_cl_cropWindow[3]);
	}

	// Pass in specified resolution.
	if(g_cl_res.size() == 2)
	{
		RiFormat(g_cl_res[0], g_cl_res[1], 1.0f);
	}

#if ENABLE_MPDUMP
	// Pass the statistics option onto Aqsis.
	if ( g_cl_mpdump )
	{
		RtInt enabled = 1;
		RiOption( "mpdump", "enabled", &enabled, RI_NULL );
	}
#endif

	for( ArgParse::apstringvec::iterator i = g_cl_options.begin(); i != g_cl_options.end(); ++i )
	{
		// This is not pretty, gzopen attempts to read a gzip header
		//from the stream, if we try and do this before we have sent
		//the request the we get deadlock, so we have to create the decoder
		//after the request, we use a buffer size of one to prevent any
		//potential blocking.
		librib::CqRibBinaryDecoder *decoder;

		int hpipe[2];
		FILE *readPipe, *writePipe;

#ifdef AQSIS_SYSTEM_WIN32

		if( _pipe( hpipe, i->length(), _O_TEXT) != -1 )
#else

		if( pipe( hpipe) != -1 )
#endif

		{
			// Feed the string contents into the pipe so that the parser can read them out the other end.
#ifdef AQSIS_SYSTEM_WIN32
			readPipe = _fdopen(hpipe[0],"r");
			writePipe = _fdopen(hpipe[1],"w");
#else

			readPipe = fdopen(hpipe[0],"r");
			writePipe = fdopen(hpipe[1],"w");
#endif

			size_t len_written = fwrite(i->c_str(),1,i->length(),writePipe);
			if(len_written != i->length())
				throw(Aqsis::XqException("Error forwarding pipe data"));
			fflush(writePipe);
			fclose(writePipe);

			decoder = new librib::CqRibBinaryDecoder( readPipe, 16384);

			// Parse the resulting block of RIB.
			librib::CqRIBParserState currstate = librib::GetParserState();

			if( currstate.m_pParseCallbackInterface == NULL )
				currstate.m_pParseCallbackInterface = new librib2ri::Engine;

			librib::ParseOpenStream( decoder, "aqsis command line option", *(currstate.m_pParseCallbackInterface), *(currstate.m_pParseErrorStream), (RtArchiveCallback) RI_NULL );

			librib::SetParserState( currstate );

			delete( decoder );

			fclose(readPipe);
		}
	}

	return ;
}


#ifndef AQSIS_SYSTEM_WIN32

static void SetPriority(int priority)
{
	pid_t pid = getpid();

        switch(priority)
	{
	case 0:
		{
		setpriority(PRIO_PROCESS, pid, 10);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to Idle" << std::endl;
#endif
		} break;
 
	case 1:
		{
		setpriority(PRIO_PROCESS, pid, 0);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to Normal" << std::endl;
#endif
		}
		break;
	case 2:
		{
		int high = getpriority(PRIO_PROCESS, pid);
		setpriority(PRIO_PROCESS, pid, high);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to High" << std::endl;
#endif
    
		}
		break;
	case 3:
		{
		setpriority(PRIO_PROCESS, pid, -20);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to RT" << std::endl;
#endif
		} break;
 
	default:
		{
		setpriority(PRIO_PROCESS, pid, 10);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to Idle" << std::endl;
#endif
		} break;
	}
	 
}
#else
static void SetPriority(int priority)
{
	switch (priority)
	{
	case 0:
		{
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to Idle" << std::endl;
#endif
		} break;
 
	case 1:
		{
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to Normal" << std::endl;
#endif
		}
		break;
	case 2:
		{
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to High" << std::endl;
#endif
    
		}
		break;
	case 3:
		{
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to RT" << std::endl;
#endif
		} break;
 
	default:
		{
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

#if defined(_DEBUG)
		std::cout << "Set Priority Class to Idle" << std::endl;
#endif
		} break;
	}
}
#endif

int main( int argc, const char** argv )
{
	StartMemoryDebugging();
	{
		ArgParse ap;
		ap.usageHeader( ArgParse::apstring( "Aqsis command line renderer\nUsage: " ) + argv[ 0 ] + " [options] [RIB file...]" );
		ap.argFlag( "help", "\aPrint this help and exit", &g_cl_help );
		ap.alias( "help" , "h" );
		ap.argFlag( "version", "\aPrint version information and exit", &g_cl_version );
		ap.argFlag( "pause", "\aWait for a keypress on completion", &g_cl_pause );
		ap.argFlag( "progress", "\aPrint progress information", &g_cl_progress );
		ap.argFlag( "Progress", "\aPrint prman-compatible progress information (ignores -progressformat)", &g_cl_Progress );
		ap.argString( "progressformat", "=string\aprintf-style format string for -progress", &g_cl_strprogress );
		ap.argInt( "endofframe", "=integer\aEquivalent to \"endofframe\" RIB option", &g_cl_endofframe );
		ap.argFlag( "nostandard", "\aDo not declare standard RenderMan parameters", &g_cl_nostandard );
		ap.argInt( "verbose", "=integer\aSet log output level\n"
		           "\a0 = errors\n"
		           "\a1 = warnings (default)\n"
		           "\a2 = information\n"
		           "\a3 = debug", &g_cl_verbose );
		ap.alias( "verbose", "v" );
		ap.argFlag( "echoapi", "\aEcho all RI API calls to the log output (experimental)", &g_cl_echoapi);

      		ap.argInt( "priority", "=integer\aControl the priority class of aqsis.\n"
         		"\a0 = idle\n"
         		"\a1 = normal(default)\n"
         		"\a2 = high\n"
         		"\a3 = RT", &g_cl_priority);
      		ap.alias( "priority", "z");

		ap.argString( "type", "=string\aSpecify a display device type to use", &g_cl_type );
		ap.argString( "addtype", "=string\aSpecify a display device type to add", &g_cl_addtype );
		ap.argString( "mode", "=string\aSpecify a display device mode to use", &g_cl_mode );
		ap.argFlag( "fb", "\aSame as --type=\"framebuffer\" --mode=\"rgb\"", &g_cl_fb );
		ap.alias( "fb", "d" );
		ap.argFloats( "crop", " x1 x2 y1 y2\aSpecify a crop window, values are in screen space.", &g_cl_cropWindow, ArgParse::SEP_ARGV, 4);
		ap.argInts( "frames", " f1 f2\aSpecify a starting/ending frame to render (inclusive).", &g_cl_frames, ArgParse::SEP_ARGV, 2);
		ap.argString( "frameslist", "=string\aSpecify a range of frames to render, ',' separated with '-' to indicate ranges.", &g_cl_framesList);
		ap.argFlag( "nocolor", "\aDisable colored output", &g_cl_no_color );
		ap.argFlag( "beep", "\aBeep on completion of all ribs", &g_cl_beep );
		ap.alias( "nocolor", "nc" );
		ap.argInts( "res", " x y\aSpecify the resolution of the render.", &g_cl_res, ArgParse::SEP_ARGV, 2);
		ap.argStrings( "option", "=string\aA valid RIB Option string, can be specified multiple times.", &g_cl_options);
#ifdef	AQSIS_SYSTEM_POSIX
		ap.argFlag( "syslog", "\aLog messages to syslog", &g_cl_syslog );
#endif	// AQSIS_SYSTEM_POSIX
#if	ENABLE_MPDUMP
		ap.argFlag( "mpdump", "\aOutput MP list to a custom 'dump' file", &g_cl_mpdump );
#endif	// ENABLE_MPDUMP

		ap.argString( "rc", "=string\aOverride the default RIB configuration file", &g_cl_rc_path );
		ap.argString( "shaders", "=string\aOverride the default shader searchpath(s)", &g_cl_shader_path );
		ap.argString( "archives", "=string\aOverride the default archive searchpath(s)", &g_cl_archive_path );
		ap.argString( "textures", "=string\aOverride the default texture searchpath(s)", &g_cl_texture_path );
		ap.argString( "displays", "=string\aOverride the default display searchpath(s)", &g_cl_display_path );
		ap.argString( "procedurals", "=string\aOverride the default procedural searchpath(s)", &g_cl_procedural_path );
		ap.argString( "plugins", "=string\aOverride the default plugin searchpath(s)", &g_cl_plugin_path );
		ap.allowUnrecognizedOptions();

		//_crtBreakAlloc = 1305;

		if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
		{
			Aqsis::log() << ap.errmsg() << std::endl << ap.usagemsg();
			exit( 1 );
		}

		// Check that the number of arguments to crop are valid if specified.
		if ( g_cl_cropWindow.size() > 0 && g_cl_cropWindow.size() != 4 )
		{
			std::cout << "Error: invalid number of arguments to -crop, expected 4, got " << g_cl_cropWindow.size() << std::endl;
			g_cl_help = true;
		}

		if ( g_cl_help )
		{
			std::cout << ap.usagemsg();
			exit( 0 );
		}

      		/* Set the priority on the main thread;
      		* gentile for single CPU XP/OS
      		*/

		if (g_cl_priority != 1)
		{
			SetPriority(g_cl_priority);
		}

		if ( g_cl_version )
		{
			std::cout << "aqsis version " << VERSION_STR_PRINT
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

		if ( ap.leftovers().size() == 0 )     // If no files specified, take input from stdin.
		{
			std::string name("stdin");
			RenderFile( stdin, name );
		}
		else
		{
			for ( ArgParse::apstringvec::const_iterator e = ap.leftovers().begin(); e != ap.leftovers().end(); e++ )
			{
				FILE *file = fopen( e->c_str(), "rb" );
				if ( file != NULL )
				{
					std::string name(*e);
					RenderFile( file, name );
					fclose( file );
				}
				else
				{
					std::cout << "Warning: Cannot open file \"" << *e << "\"" << std::endl;
				}
			}
		}

	}

	StopMemoryDebugging();

	if(g_cl_beep)
	{
		std::cout << "\a" << std::ends;
	}

	if(g_cl_pause)
	{
		std::cout << "Press any key..." << std::ends;
		std::cin.ignore(std::cin.rdbuf()->in_avail() + 1);
	}

	return ( 0 );
}

/**
  Render a RIB file.

  This function reads RIB requests from the open file \a file and
  renders the contents of the file. It is the callers responsibility
  to close the file after the function has returned.

  The RIB file is parsed by the \link librib librib \endlink library and 
  the RIB requests are processed by the \link librib2ri::Engine 
  RendermanInterface \endlink object defined in the librib2ri library 
  (as returned by librib2ri::CreateRIBEngine()).
   
  \param file An open file handle
  \param name The file name
 */
void RenderFile( FILE* file, std::string&  name )
{
	librib::RendermanInterface * renderengine = librib2ri::CreateRIBEngine();

	try
	{
		RiBegin( "aqsis" );

		if ( !g_cl_nostandard )
			librib::StandardDeclarations( renderengine );

		if ( g_cl_echoapi )
		{
			RtInt echoapi = 1;
			RiOption( "statistics", "echoapi", &echoapi, RI_NULL );
		}

		/* Allow any command line arguments to override system/env settings */
		const char* popt[1];
		Aqsis::log() << Aqsis::info << "Applying search paths provided at the command line" << std::endl;
		if(!g_cl_shader_path.empty())
		{
			popt[0] = g_cl_shader_path.c_str();
			RiOption( "searchpath", "shader", &popt, RI_NULL );
		}
		if(!g_cl_archive_path.empty())
		{
			popt[0] = g_cl_archive_path.c_str();
			RiOption( "searchpath", "archive", &popt, RI_NULL );
		}
		if(!g_cl_texture_path.empty())
		{
			popt[0] = g_cl_texture_path.c_str();
			RiOption( "searchpath", "texture", &popt, RI_NULL );
		}
		if(!g_cl_display_path.empty())
		{
			popt[0] = g_cl_display_path.c_str();
			RiOption( "searchpath", "display", &popt, RI_NULL );
		}
		if(!g_cl_procedural_path.empty())
		{
			popt[0] = g_cl_procedural_path.c_str();
			RiOption( "searchpath", "procedural", &popt, RI_NULL );
		}
		if(!g_cl_plugin_path.empty())
		{
			popt[0] = g_cl_plugin_path.c_str();
			RiOption( "searchpath", "plugin", &popt, RI_NULL );
		}

		RiProgressHandler( &PrintProgress );
		RiPreWorldFunction( &PreWorld );


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

		librib::Parse( file, name, *renderengine, Aqsis::log(), NULL );

		RiEnd();

		if ( !g_cl_nostandard )
			librib::CleanupDeclarations( *renderengine );
	}
	catch(Aqsis::XqException& x)
	{
		Aqsis::log() << Aqsis::error << x.what() << std::endl;
	}

	librib2ri::DestroyRIBEngine( renderengine );
}
