#include <time.h>

#include "ri.h"
#include "librib.h"
#include "librib2ri.h"
#include "aqsis.h"

#if defined(AQSIS_SYSTEM_WIN32)
#include <windows.h>
#endif

#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
#include "version.h"
#endif

#if defined(AQSIS_SYSTEM_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include <argparse.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <strstream>

#include <stdio.h>

#define __MESSAGE_TABLE
#include "mtable.h"
#undef __MESSAGE_TABLE

CqMessageTable mTable; // Create the message table


void RenderFile( FILE* file, const char* name );
void GetOptions();

bool g_pause;
#ifdef	_DEBUG
int g_endofframe = 3;
#else
int g_endofframe = 0;
#endif
bool g_nostandard = 0;
bool g_help = 0;
bool g_version = 0;
bool g_verbose = 0;
bool g_fb = 0;
bool g_d = 0;
bool g_progress = 0;
ArgParse::apstring g_config = "";
ArgParse::apstring g_shaders = "";
ArgParse::apstring g_archives = "";
ArgParse::apstring g_textures = "";
ArgParse::apstring g_displays = "";
ArgParse::apstring g_base_path = "";
ArgParse::apstring g_type = "";
ArgParse::apstring g_addtype = "";
ArgParse::apstring g_mode = "rgba";
ArgParse::apstring g_strprogress = "Done Computing %p%% [ %s secs / %S left ]";

void version( std::ostream& Stream )
{
#if defined(AQSIS_SYSTEM_WIN32) || defined(AQSIS_SYSTEM_MACOSX)
	Stream << "aqsis version " << VERSION_STR << std::endl;
#else
	Stream << "aqsis version " << VERSION << std::endl;
#endif
}


/** Function to print the progress of the render.
	Used as the callback function to a RiProgressHandler call.
 */
RtVoid PrintProgress( RtFloat percent )
{
	if ( g_progress == 0 )
		return ;

	if ( percent > 100 )
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
#ifdef	AQSIS_SYSTEM_POSIX
	TqFloat total_secs = ( RtFloat ) 1000000.0f * ( ( RtFloat ) ( now - tick ) / ( float ) CLOCKS_PER_SEC );
#elif AQSIS_SYSTEM_MACOSX
	TqFloat total_secs = ( RtFloat ) 100.0f * ( ( RtFloat ) ( now - tick ) / ( float ) CLOCKS_PER_SEC );
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
	std::string strProgress( g_strprogress );
	std::ostrstream strOutput;
	while ( 1 )
	{
		TqInt itag;
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
					strOutput << std::setw( 6 ) << std::setfill( ' ' ) << std::setprecision( 4 ) << percent;
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
	while ( strOutput.pcount() < 79 )
		strOutput << " ";
	std::cout << std::string( strOutput.str(), strOutput.pcount() ).c_str() << "\r";
	strOutput.freeze(false);
	std:: cout << std::flush;
}


/** Function to setup specific options needed after world loading but before rendering.
	Used as the callback function to a RiPreRenderFunction call.
 */
#ifdef	AQSIS_SYSTEM_BEOS
RtVoid PreRender( ... )
#else
RtVoid PreRender()
#endif
{
	if ( g_fb || g_d )
	{
		char * type = "framebuffer", *mode = "rgba";
		RiDisplay( "aqsis", type, mode, NULL );
	}
	else if ( g_type.compare( "" ) != 0 )
	{
		char type[ 256 ], mode[ 256 ];
		strcpy( type, g_type.c_str() );
		strcpy( mode, g_mode.c_str() );
		RiDisplay( "aqsis", type, mode, NULL );
	}
	else if ( g_addtype.compare( "" ) != 0 )
	{
		char type[ 256 ], mode[ 256 ];
		strcpy( type, g_addtype.c_str() );
		strcpy( mode, g_mode.c_str() );
		RiDisplay( "+aqsis", type, mode, NULL );
	}
	return ;
}


int main( int argc, const char** argv )
{

	ArgParse ap;
	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options] files(s) to render" );
	ap.argFlag( "help", "\aprint this help and exit", &g_help );
	ap.argFlag( "version", "\aprint version information and exit", &g_version );
	ap.argFlag( "pause", "\await for a keypress on completion", &g_pause );
	ap.argFlag( "progress", "\aprint progress information", &g_progress );
	ap.argString( "progressformat", "\astring representing the format of the progress message", &g_strprogress );
	ap.argInt( "endofframe", "=integer\aequivalent to \"endofframe\" option", &g_endofframe );
	ap.argFlag( "nostandard", "\adisables declaration of standard RenderMan parameter types", &g_nostandard );
	ap.argFlag( "verbose", "\aoutput environment information", &g_verbose );
	ap.argString( "type", "=string\aspecify a display device type to use", &g_type );
	ap.argString( "addtype", "=string\aspecify a display device type to add", &g_addtype );
	ap.argString( "mode", "=string\aspecify a display device mode to use", &g_mode );
	ap.argFlag( "fb", "\aequivalent to --type=\"framebuffer\" --mode=\"rgba\"", &g_fb );
	ap.argFlag( "d", "\aequivalent to -fb", &g_d );
	ap.argString( "config", "=string\aspecify a configuration file to load", &g_config );
	ap.argString( "base", "=string\aspecify a default base path", &g_base_path );
	ap.argString( "shaders", "=string\aspecify a default shaders searchpath", &g_shaders );
	ap.argString( "archives", "=string\aspecify a default archives searchpath", &g_archives );
	ap.argString( "textures", "=string\aspecify a default textures searchpath", &g_textures );
	ap.argString( "displays", "=string\aspecify a default displays searchpath", &g_displays );

	//_crtBreakAlloc = 845;
	
	if ( argc > 1 && !ap.parse( argc - 1, argv + 1 ) )
	{
		std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
		exit( 1 );
	}

	if ( g_help )
	{
		std::cout << ap.usagemsg();
		exit( 0 );
	}

	if ( g_version )
	{
		version( std::cout );
		std::cout << "compiled " << __DATE__ << " " << __TIME__  << std::endl;
		exit( 0 );
	}

	GetOptions();

	if ( g_verbose )
	{
		std::cout << "config:   " << g_config.c_str() << std::endl;
		std::cout << "base:     " << g_base_path.c_str() << std::endl;
		std::cout << "shaders:  " << g_shaders.c_str() << std::endl;
		std::cout << "archives: " << g_archives.c_str() << std::endl;
		std::cout << "textures: " << g_textures.c_str() << std::endl;
		std::cout << "displays: " << g_displays.c_str() << std::endl;
	}

	if ( ap.leftovers().size() == 0 )    // If no files specified, take input from stdin.
	{
		RenderFile( stdin, "stdin" );
	}
	else
	{
		for ( ArgParse::apstringvec::const_iterator e = ap.leftovers().begin(); e != ap.leftovers().end(); e++ )
		{
			FILE *file = fopen( e->c_str(), "rb" );
			if ( file != NULL )
			{
				RenderFile( file, e->c_str() );
				fclose( file );
			}
			else
			{
				std::cout << "Warning: Cannot open file \"" << *e << "\"" << std::endl;
			}
		}
	}

#if defined(AQSIS_SYSTEM_WIN32) && defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif

#if defined(AQSIS_SYSTEM_WIN32)
	{
		MEMORY_BASIC_INFORMATION mbi;
		DWORD dwMemUsed = 0;
		PVOID pvAddress = 0;
		SYSTEM_INFO SystemInfo;

		memset( &SystemInfo, 0, sizeof( SYSTEM_INFO ) );
		GetSystemInfo(
		    &SystemInfo  // system information
		);
		memset( &mbi, 0, sizeof( MEMORY_BASIC_INFORMATION ) );
		while ( VirtualQuery( pvAddress, &mbi, sizeof( MEMORY_BASIC_INFORMATION ) ) == sizeof( MEMORY_BASIC_INFORMATION ) )
		{
			if ( mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE )
				dwMemUsed += mbi.RegionSize;
			pvAddress = ( ( BYTE* ) mbi.BaseAddress ) + mbi.RegionSize;
		}
		std::cout << "Peek Memory Used " << dwMemUsed << std::endl;

	}
#endif

	return ( 0 );
}


void GetOptions()
{
	char * env;
	// If --base not specified, check for env.
	if ( g_base_path.compare( "" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_BASE_PATH" ) ) != NULL )
			g_base_path = env;
#ifdef	AQSIS_SYSTEM_POSIX
		else
			g_base_path = BASE_PATH;
#endif

	}

	// If --config not specified try to locate the config file.
	if ( g_config.compare( "" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_CONFIG" ) ) != NULL )
			g_config = env;
		else
		{
#ifdef	AQSIS_SYSTEM_POSIX
			g_config = CONFIG_PATH;
#else
			g_config = g_base_path;
#endif
			g_config.append( "/.aqsisrc" );
		}

		std::ifstream cfgfile( g_config.c_str() );
		if ( !cfgfile.is_open() )
			if ( ( env = getenv( "HOME" ) ) != NULL )
			{
				g_config = env;
				g_config.append( "/.aqsisrc" );
				std::ifstream cfgfile( g_config.c_str() );
				if ( !cfgfile.is_open() )
				{
					g_config = "/etc/.aqsisrc";
				}
			}
	}

	// if --shaders is not specified, try and get a default shaders searchpath.
	if ( g_shaders.compare( "" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_SHADERS_PATH" ) ) != 0 )
			g_shaders = env;
		else
		{
			g_shaders = g_base_path;
			g_shaders.append( "/shaders" );
		}
	}

	// if --archives is not specified, try and get a default archives searchpath.
	if ( g_archives.compare( "" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_ARCHIVES_PATH" ) ) != 0 )
			g_archives = env;
		else
		{
			g_archives = g_base_path;
			g_archives.append( "/archives" );
		}
	}

	// if --textures is not specified, try and get a default textures searchpath.
	if ( g_textures.compare( "" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_TEXTURES_PATH" ) ) != 0 )
			g_textures = env;
		else
		{
			g_textures = g_base_path;
			g_textures.append( "/textures" );
		}
	}

	// if --displays is not specified, try and get a default displays searchpath.
	if ( g_displays.compare( "" ) == 0 )
	{
		if ( ( env = getenv( "AQSIS_DISPLAYS_PATH" ) ) != 0 )
			g_displays = env;
		else
		{
			g_displays = g_base_path;
			g_displays.append( "/displays" );
		}
	}
}

void RenderFile( FILE* file, const char* name )
{
	librib2ri::Engine renderengine;

	RiBegin( "CRIBBER" );

	if ( !g_nostandard )
		librib::StandardDeclarations( renderengine );

	RiOption( "statistics", "endofframe", &g_endofframe, RI_NULL );
	const char* popt[ 1 ];
	popt[ 0 ] = g_shaders.c_str();
	RiOption( "searchpath", "shader", &popt, RI_NULL );
	popt[ 0 ] = g_archives.c_str();
	RiOption( "searchpath", "archive", &popt, RI_NULL );
	popt[ 0 ] = g_textures.c_str();
	RiOption( "searchpath", "texture", &popt, RI_NULL );
	popt[ 0 ] = g_displays.c_str();
	RiOption( "searchpath", "display", &popt, RI_NULL );
	librib::UpdateArchivePath( g_archives );

	RiProgressHandler( &PrintProgress );
	RiPreRenderFunction( &PreRender );

	if ( g_config.compare( "" ) )
	{
		FILE * cfgfile = fopen( g_config.c_str(), "rb" );
		if ( cfgfile != NULL )
		{
			librib::Parse( cfgfile, "config", renderengine, std::cerr );
			fclose( cfgfile );
		}
		else if ( g_verbose )
		{
#ifdef  AQSIS_SYSTEM_WIN32
			std::cout << "Warning: Config file not found in" << std::endl <<
			"%AQSIS_CONFIG%" << std::endl <<
			"%AQSIS_BASE_PATH%/.aqsisrc" << std::endl <<
			"%HOME%/.aqsisrc" << std::endl <<
			"/etc/.aqsisrc" << std::endl;
#else
			std::cout << "Warning: Config file not found in" << std::endl <<
			"$AQSIS_CONFIG" << std::endl <<
			"$AQSIS_BASE_PATH/.aqsisrc" << std::endl <<
			"$HOME/.aqsisrc" << std::endl <<
			"/etc/.aqsisrc" << std::endl;
#endif

		}
	}
	librib::Parse( file, name, renderengine, std::cerr );

	RiEnd();
}
