// Aqsis
// Copyright (C) 1997 - 2001, Paul C. Gregory
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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/aqsis.h>

#include	<string>
#include	<list>
#include	<float.h>

#include	<boost/thread/thread.hpp>
#include	<boost/thread/mutex.hpp>
#include	<boost/archive/iterators/binary_from_base64.hpp>
#include	<boost/archive/iterators/transform_width.hpp>
#include	<boost/archive/iterators/remove_whitespace.hpp>

#include    <boost/version.hpp>

#if BOOST_VERSION < 103700
#   include	<boost/pfto.hpp>
#else
#   include	<boost/serialization/pfto.hpp>
#endif

#ifndef AQSIS_SYSTEM_WIN32
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <fcntl.h>
#	include <errno.h>
#else
#	include <winsock2.h>
#endif
#include	<tinyxml.h>

#include	<aqsis/util/argparse.h>
#include	<aqsis/util/logging.h>
#include	<aqsis/util/logging_streambufs.h>
#include	<aqsis/util/sstring.h>
#include	<aqsis/util/socket.h>
#include	<aqsis/version.h>

#include	"piqsl_ui.h"
#include	"displayserverimage.h"

using namespace Aqsis;

ArgParse::apstring      g_strInterface = "127.0.0.1";
ArgParse::apstring      g_strPort = "49515";
bool    		g_fHelp = 0;
bool    		g_fVersion = 0;
bool 			g_cl_no_color = false;
bool 			g_cl_syslog = false;
ArgParse::apint 	g_cl_verbose = 1;


void version( std::ostream& Stream )
{
	Stream << "piqsl version " << AQSIS_VERSION_STR_FULL << std::endl << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
}


int main( int argc, char** argv )
{
	ArgParse ap;

	// Set up the options
	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options] [BKS file...] [Image file...]" );
	ap.argString( "i", "\aSpecify the IP address to listen on (default: %default)", &g_strInterface );
	ap.argString( "p", "\aSpecify the TCP port to listen on (default: %default)", &g_strPort );
	ap.argFlag( "help", "\aPrint this help and exit", &g_fHelp );
	ap.alias( "help" , "h" );
	ap.argFlag( "version", "\aPrint version information and exit", &g_fVersion );
	ap.argFlag( "nocolor", "\aDisable colored output", &g_cl_no_color );
	ap.alias( "nocolor", "nc" );
	ap.argInt( "verbose", "=integer\aSet log output level\n"
		"\a0 = errors\n"
		"\a1 = warnings (default)\n"
		"\a2 = information\n"
		"\a3 = debug", &g_cl_verbose );
	ap.alias( "verbose", "v" );

#ifdef  AQSIS_SYSTEM_POSIX
        ap.argFlag( "syslog", "\aLog messages to syslog", &g_cl_syslog );
#endif  // AQSIS_SYSTEM_POSIX

    QApplication app(argc, argv);

	const char **c_argv = const_cast<const char**>(argv);
	if ( argc > 1 && !ap.parse( argc - 1, c_argv + 1 ) )
	{
		std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
 		exit( 1 );
	}

	if ( g_fHelp)
	{
		std::cerr << ap.usagemsg();
 		exit( 0 );
	}

	if ( g_fVersion )
	{
		version( std::cout );
		exit( 0 );
	}

#ifdef  AQSIS_SYSTEM_WIN32
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
	if( g_cl_verbose > 0 )
		level = Aqsis::WARNING;
	if( g_cl_verbose > 1 )
		level = Aqsis::INFO;
	if( g_cl_verbose > 2 )
		level = Aqsis::DEBUG;
	std::auto_ptr<std::streambuf> filter_level( new Aqsis::filter_by_level_buf(level, Aqsis::log()) );

#ifdef  AQSIS_SYSTEM_POSIX
	if( g_cl_syslog )
		std::auto_ptr<std::streambuf> use_syslog( new Aqsis::syslog_buf(std::cerr) );
#endif  // AQSIS_SYSTEM_POSIX

	int portno = atoi(g_strPort.c_str());
	CqSocket::initialiseSockets();
	if(g_theSocket.prepare(g_strInterface, portno))
		Fl::add_fd(g_theSocket,&HandleConnection);
	else
		Aqsis::log() << Aqsis::error << "Cannot open server on the specified port" << std::endl;

	window = new CqPiqslMainWindow(640, 480, "Piqsl");
	char arg1[] = "piqsl";
	char* internalArgs[] = {arg1};
	window->show(1, internalArgs);


	// Take the leftovers and open either a book or a tiff
	for ( ArgParse::apstringvec::const_iterator e = ap.leftovers().begin();
	      e != ap.leftovers().end(); e++ )
	{
		FILE *file = fopen( e->c_str(), "rb" );
		if ( file != NULL )
		{
			fclose( file );
			std::string name(*e);
			TiXmlDocument doc(name);
			bool loadOkay = doc.LoadFile();
			if(loadOkay)
			{
				// Load a booklet
				window->loadConfiguration(name);
			} else 
			{
				// load one image
				boost::shared_ptr<CqImage> newImage(new CqImage(name));
				newImage->loadFromFile(name);
				window->addImageToCurrentBook(newImage);
			}
		}
		else
		{
			std::cout << "Warning: Cannot open file \"" << *e << "\"" << std::endl;
		}
	}

	int result = 0;
	for(;;)
	{
		Fl::wait();
		// Act upon an resize/update requests on the framebuffers.
		window->checkResize();
		if(Fl::first_window() == NULL)
			break;
	}

	return(result);
}


