// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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
#include "argparse.h"
#include "file.h"
#include "librib.h"
#include "librib2ri.h"
#include "logging.h"
#include "logging_streambufs.h"
#include "ri.h"
#include "version.h"

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

// Storage for the actual resource paths that we will use at runtime
std::string g_rc_path;
std::string g_shader_path;
std::string g_archive_path;
std::string g_texture_path;
std::string g_display_path;
std::string g_dso_path;
std::string g_procedural_path;

// Forward declarations
void RenderFile( FILE* file, std::string& name );

// Command-line arguments
bool g_cl_pause;
// Set verbose stats if in debug mode
#ifdef	_DEBUG
int g_cl_endofframe = 3;
#else
int g_cl_endofframe = -1;
#endif
bool g_cl_nostandard = 0;
bool g_cl_help = 0;
bool g_cl_version = 0;
bool g_cl_fb = 0;
bool g_cl_progress = 0;
bool g_cl_Progress = 0;
bool g_cl_rinfo = 0;
bool g_cl_no_color = 0;
int g_cl_verbose = 1;
ArgParse::apfloatvec g_cl_cropWindow;
ArgParse::apstring g_cl_rc_path = "";
ArgParse::apstring g_cl_shader_path = "";
ArgParse::apstring g_cl_archive_path = "";
ArgParse::apstring g_cl_texture_path = "";
ArgParse::apstring g_cl_display_path = "";
ArgParse::apstring g_cl_dso_path = "";
ArgParse::apstring g_cl_procedural_path = "";
ArgParse::apstring g_cl_type = "";
ArgParse::apstring g_cl_addtype = "";
ArgParse::apstring g_cl_mode = "rgba";
ArgParse::apstring g_cl_strprogress = "Frame (%f) %p%% complete [ %s secs / %S left ]";

#ifdef	AQSIS_SYSTEM_POSIX
bool g_cl_syslog = 0;
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

    static long tick = 0;
    long now;

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
    TqInt ipos = 0;

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
    strOutput.setf( std::ios::fixed );
    while ( 1 )
    {
        TqUint itag;
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


/** Function to setup specific options needed after world loading but before rendering.
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
    return ;
}


int main( int argc, const char** argv )
{
// Set default resource paths ...
#ifdef AQSIS_SYSTEM_WIN32
	char acPath[256];
	char rootPath[256];
	if( GetModuleFileName( NULL, acPath, 256 ) != 0) 
	{
		// guaranteed file name of at least one character after path
		*( strrchr( acPath, '\\' ) + 1 ) = '\0';
		std::string	 stracPath(acPath);
		stracPath.append("..\\");
		_fullpath(rootPath,&stracPath[0],256);
	}
	g_rc_path = rootPath;
	g_shader_path = rootPath;
	g_archive_path = rootPath;
	g_texture_path = rootPath;
	g_display_path = rootPath;
	g_dso_path = rootPath;
	g_procedural_path = rootPath;

	g_shader_path.append( "shaders" );
	g_archive_path.append( "archives" );
	g_texture_path.append( "textures" );
	g_display_path.append( "bin" );
	g_dso_path.append( "dsos" );
	g_procedural_path.append( "procedures" );
#else
	g_rc_path = DEFAULT_RC_PATH;
	g_shader_path = DEFAULT_SHADER_PATH;
	g_archive_path = DEFAULT_ARCHIVE_PATH;
	g_texture_path = DEFAULT_TEXTURE_PATH;
	g_display_path = DEFAULT_DISPLAY_PATH;
	g_dso_path = DEFAULT_DSO_PATH;
	g_procedural_path = DEFAULT_PROCEDURAL_PATH;
#endif

    StartMemoryDebugging();
    {
        ArgParse ap;
        ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options] [RIB file...]" );
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
        ap.argFlag( "renderinfo", "\aPrint out infos about base rendering settings", &g_cl_rinfo );
        ap.argString( "type", "=string\aSpecify a display device type to use", &g_cl_type );
        ap.argString( "addtype", "=string\aSpecify a display device type to add", &g_cl_addtype );
        ap.argString( "mode", "=string\aSpecify a display device mode to use", &g_cl_mode );
        ap.argFlag( "fb", "\aSame as --type=\"framebuffer\" --mode=\"rgb\"", &g_cl_fb );
        ap.alias( "fb", "d" );
        ap.argFloats( "crop", " x1 x2 y1 y2\aSpecify a crop window, values are in screen space.", &g_cl_cropWindow, ArgParse::SEP_ARGV, 4);
        ap.argString( "rc", "=string\aOverride the default RIB configuration file [" + g_rc_path + "]", &g_cl_rc_path );
        ap.argString( "shaders", "=string\aOverride the default shader searchpath(s) [" + g_shader_path + "]", &g_cl_shader_path );
        ap.argString( "archives", "=string\aOverride the default archive searchpath(s) [" + g_archive_path + "]", &g_cl_archive_path );
        ap.argString( "textures", "=string\aOverride the default texture searchpath(s) [" + g_texture_path + "]", &g_cl_texture_path );
        ap.argString( "displays", "=string\aOverride the default display searchpath(s) [" + g_display_path + "]", &g_cl_display_path );
        ap.argString( "dsolibs", "=string\aOverride the default dso searchpath(s) [" + g_dso_path + "]", &g_cl_dso_path );
        ap.argString( "procedurals", "=string\aOverride the default procedural searchpath(s) [" + g_procedural_path + "]", &g_cl_procedural_path );
        ap.argFlag( "nocolor", "\aDisable colored output", &g_cl_no_color );
        ap.alias( "nocolor", "nc" );
#ifdef	AQSIS_SYSTEM_POSIX
        ap.argFlag( "syslog", "\aLog messages to syslog", &g_cl_syslog );
#endif	// AQSIS_SYSTEM_POSIX
        ap.allowUnrecognizedOptions();

        //_crtBreakAlloc = 1305;

        if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
        {
            std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
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

        if ( g_cl_version )
        {
            std::cout << "aqsis version " << VERSION_STR << std::endl;
            std::cout << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
            exit( 0 );
        }

        // Apply environment-variable overrides to default paths ...
	if(getenv("AQSIS_RC_PATH"))
		g_rc_path = getenv("AQSIS_RC_PATH");
	if(getenv("AQSIS_SHADER_PATH"))
		g_shader_path = getenv("AQSIS_SHADER_PATH");
	if(getenv("AQSIS_ARCHIVE_PATH"))
		g_archive_path = getenv("AQSIS_ARCHIVE_PATH");
	if(getenv("AQSIS_TEXTURE_PATH"))
		g_texture_path = getenv("AQSIS_TEXTURE_PATH");
	if(getenv("AQSIS_DISPLAY_PATH"))
		g_display_path = getenv("AQSIS_DISPLAY_PATH");
	if(getenv("AQSIS_DSO_PATH"))
		g_dso_path = getenv("AQSIS_DSO_PATH");
	if(getenv("AQSIS_PROCEDURAL_PATH"))
		g_procedural_path = getenv("AQSIS_PROCEDURAL_PATH");

        // Apply command-line overrides to default paths ...
        if(!g_cl_rc_path.empty())
            g_rc_path = g_cl_rc_path;
        if(!g_cl_shader_path.empty())
            g_shader_path = g_cl_shader_path;
        if(!g_cl_archive_path.empty())
            g_archive_path = g_cl_archive_path;
        if(!g_cl_texture_path.empty())
            g_texture_path = g_cl_texture_path;
        if(!g_cl_display_path.empty())
            g_display_path = g_cl_display_path;
        if(!g_cl_dso_path.empty())
            g_dso_path = g_cl_dso_path;
        if(!g_cl_procedural_path.empty())
            g_procedural_path = g_cl_procedural_path;

#ifdef	AQSIS_SYSTEM_WIN32
        std::auto_ptr<std::streambuf> ansi( new Aqsis::ansi_buf(std::cerr) );
#endif
        std::auto_ptr<std::streambuf> reset_level( new Aqsis::reset_level_buf(std::cerr) );
        std::auto_ptr<std::streambuf> show_timestamps( new Aqsis::timestamp_buf(std::cerr) );
        std::auto_ptr<std::streambuf> fold_duplicates( new Aqsis::fold_duplicates_buf(std::cerr) );
        std::auto_ptr<std::streambuf> color_level;
        if(!g_cl_no_color)
        {
            std::auto_ptr<std::streambuf> temp_color_level( new Aqsis::color_level_buf(std::cerr) );
            color_level = temp_color_level;
        }
        std::auto_ptr<std::streambuf> show_level( new Aqsis::show_level_buf(std::cerr) );
        Aqsis::log_level_t level = Aqsis::ERROR;
        if( g_cl_verbose > 0 )	level = Aqsis::WARNING;
        if( g_cl_verbose > 1 )	level = Aqsis::INFO;
        if( g_cl_verbose > 2 )	level = Aqsis::DEBUG;
        std::auto_ptr<std::streambuf> filter_level( new Aqsis::filter_by_level_buf(level, std::cerr) );
#ifdef	AQSIS_SYSTEM_POSIX
        if( g_cl_syslog )
            std::auto_ptr<std::streambuf> use_syslog( new Aqsis::syslog_buf(std::cerr) );
#endif	// AQSIS_SYSTEM_POSIX

	    std::cerr << Aqsis::info << "rc path: " << g_rc_path << std::endl;
	    std::cerr << Aqsis::info << "shader path(s): " << g_shader_path << std::endl;
	    std::cerr << Aqsis::info << "archive path(s): " << g_archive_path << std::endl;
	    std::cerr << Aqsis::info << "texture path(s): " << g_texture_path << std::endl;
	    std::cerr << Aqsis::info << "display path(s): " << g_display_path << std::endl;
	    std::cerr << Aqsis::info << "dso path(s): " << g_dso_path << std::endl;
	    std::cerr << Aqsis::info << "procedural path(s): " << g_procedural_path << std::endl;

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

    if(g_cl_pause)
    {
        std::cout << "Press any key..." << std::ends;
        std::cin.ignore(std::cin.rdbuf()->in_avail() + 1);
    }

    return ( 0 );
}

void RenderFile( FILE* file, std::string&  name )
{
    librib::RendermanInterface * renderengine = librib2ri::CreateRIBEngine();

    RiBegin( "CRIBBER" );

    if ( !g_cl_nostandard )
        librib::StandardDeclarations( *renderengine );

    if ( g_cl_rinfo )
    {
        RiOption( "statistics", "renderinfo", &g_cl_rinfo, RI_NULL );
    }

    const char* popt[ 1 ];
    popt[ 0 ] = g_shader_path.c_str();
    RiOption( "searchpath", "shader", &popt, RI_NULL );
    popt[ 0 ] = g_archive_path.c_str();
    RiOption( "searchpath", "archive", &popt, RI_NULL );
    popt[ 0 ] = g_texture_path.c_str();
    RiOption( "searchpath", "texture", &popt, RI_NULL );
    popt[ 0 ] = g_display_path.c_str();
    RiOption( "searchpath", "display", &popt, RI_NULL );
    popt[ 0 ] = g_dso_path.c_str();
    RiOption( "searchpath", "dsolibs", &popt, RI_NULL );
    popt[ 0 ] = g_procedural_path.c_str();
    RiOption( "searchpath", "procedural", &popt, RI_NULL );

    RiProgressHandler( &PrintProgress );
    RiPreWorldFunction( &PreWorld );

    std::cerr << Aqsis::info << "Loading config file " << g_rc_path << std::endl;
    FILE* rcfile = fopen( g_rc_path.c_str(), "rb" );
    if (rcfile != NULL )
    {
	librib::Parse( rcfile, "config", *renderengine, std::cerr, NULL );
	fclose( rcfile );
    }

    librib::Parse( file, name, *renderengine, std::cerr, NULL );

    RiEnd();

    if ( !g_cl_nostandard )
        librib::CleanupDeclarations( *renderengine );

    librib2ri::DestroyRIBEngine( renderengine );
}
