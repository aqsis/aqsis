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
		\brief Implements the default display devices for Aqsis.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include <aqsis.h>

#include "argparse.h"

#include <logging.h>
#include <logging_streambufs.h>
#include "sstring.h"

#include <tiffio.h>

using namespace Aqsis;

#include <string>
#include <list>

#include <version.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "eqshibit.h"
#include "eqshibit_ui.h"
#include "ddserver.h"
#include "ddclient.h"
#include "displaydriver.h"

#define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define INT_PRELERP(p, q, a, t) ( (p) + (q) - INT_MULT( a, p, t) )

ArgParse::apstring      g_strInterface = "127.0.0.1";
ArgParse::apstring      g_strPort = "48515";
bool    		g_fHelp = 0;
bool    		g_fVersion = 0;
bool 			g_cl_no_color = false;
bool 			g_cl_syslog = false;
ArgParse::apint 	g_cl_verbose = 1;

CqEqshibitMainWindow *window = 0;

void version( std::ostream& Stream )
{
	Stream << "eqshibit version " << VERSION_STR_PRINT << std::endl;
}

CqDDServer g_theServer;
std::map<int, CqDDClient> g_theClients;

void HandleData(int sock, void *data)
{
	CqDDClient thisClient = g_theClients[sock];
	SqDDMessageBase msg;
	char	*bptr;
	int	i, numRead=0;

	bptr = reinterpret_cast<char*>(&msg);
	while(numRead < sizeof(msg))	
	{
		i = read(sock,bptr,sizeof(msg) - numRead);
		if(i<=0) 
			break;
		bptr += i;
		numRead += i;
	}
	if(numRead == sizeof(msg))
	{
		std::cout << "Message ID: " << msg.m_MessageID << "(" << std::hex << msg.m_MessageID << ")" << std::dec << " length: " << msg.m_MessageLength << std::endl;
		int msgToRead = msg.m_MessageLength - numRead;
		char *buff = new char[msgToRead];
		bptr = buff;
		numRead = 0;
		while(numRead < msgToRead)
		{
			i = read(sock,bptr,msgToRead - numRead);
			if(i<=0) 
				break;
			bptr += i;
			numRead += i;
		}
		delete[](buff);
		std::cout << "Read: " << numRead << std::endl;
		if(msg.m_MessageID == MessageID_Close)
		{
			std::cout << "Closing socket" << std::endl;
			Fl::remove_fd(sock);
			close(sock);
		}
	}		
}

void HandleConnection(int sock, void *data)
{
	Aqsis::log() << Aqsis::info << "Connection established with display server" << std::endl;

	CqDDClient newClient;
	
	if(g_theServer.Accept(newClient))
	{
		g_theClients[newClient.Socket()] = newClient;
		Fl::add_fd(newClient.Socket(), FL_READ, &HandleData);
		if(window)
			window->addImageToCurrentCatalog("Hello");

		Fl_Window *window = new Fl_Window(300,180);
		Fl_Box *box = new Fl_Box(20,40,260,100,"Hello, World!");
		box->box(FL_UP_BOX);
		box->labelsize(36);
		box->labelfont(FL_BOLD+FL_ITALIC);
		box->labeltype(FL_SHADOW_LABEL);
		window->end();
		window->show();
	}
}


void Fl_FrameBuffer_Widget::draw(void)
{
	fl_draw_image(image,x(),y(),w,h,d,w*d); // draw image
}


int main( int argc, char** argv )
{
	// Create listening socket. 
	// Setup fltk. 
	// add the socket to the fltk event
	// run fltk
	// blah
	ArgParse ap;

	// Set up the options
	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options]" );
	ap.argString( "i", "\aSpecify the address to listen on", &g_strInterface );
	ap.argString( "p", "\aSpecify the port to listen on", &g_strPort );
	ap.argFlag( "help", "\aprint this help and exit", &g_fHelp );
	ap.argFlag( "version", "\aprint version information and exit", &g_fVersion );
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


	const char **c_argv = const_cast<const char**>(argv);
	if ( argc > 1 && !ap.parse( argc - 1, c_argv + 1 ) )
	{
		std::cerr << ap.errmsg() << std::endl << ap.usagemsg();
 		exit( 1 );
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
	if(!g_theServer.Prepare(portno))
		Aqsis::log() << Aqsis::error << "Cannot open server on the specified port" << std::endl;

	Fl::add_fd(g_theServer.Socket(),&HandleConnection);

	window = new CqEqshibitMainWindow();
	char *internalArgs[] = {
		"eqshibit"
	};
	window->show(1, internalArgs);

	return Fl::run();
}

