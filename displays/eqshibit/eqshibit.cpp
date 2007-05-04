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
#include "displayserverimage.h"
#include "displaydriver.h"
#include "framebuffer.h"
#include "book.h"
#include "tinyxml.h"


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
	Stream << "eqshibit version " << VERSION_STR_PRINT << std::endl << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
}


CqDDServer g_theServer;
std::map<int, boost::shared_ptr<CqDisplayServerImage> >	g_theClients;
#define BUF_SIZE  4096

void HandleData(int sock, void *data)
{
	boost::shared_ptr<CqDisplayServerImage> thisClient = g_theClients[sock];
	char	buffer[BUF_SIZE+1];
	std::stringstream msg;
	unsigned int	i, numRead=0;

	// Read a message
	while(1)	
	{
		i = read(sock,buffer,BUF_SIZE);
		if(i<=0) 
			break;
		numRead += i;
		if(i == BUF_SIZE)
			buffer[BUF_SIZE] = '\0';
		msg << buffer;
		if(i<BUF_SIZE)
			break;
	}
	std::cout << "Recieved: " << numRead << " bytes" << std::endl;
	TiXmlDocument xmlMsg;
	xmlMsg.Parse(msg.str().c_str());
	TiXmlElement* root = xmlMsg.RootElement();
	std::cout << "Message: " << root->ValueStr() << std::endl;

	if(root)
	{
		if(root->ValueStr().compare("Open") == 0)
		{
			Aqsis::log() << Aqsis::debug << "Now processing the Open message" << std::endl;
#if 0
			thisClient->setImageSize(openMsg->m_XRes, openMsg->m_YRes);
			thisClient->setChannels(openMsg->m_Channels);
			thisClient->setOrigin(openMsg->m_originX, openMsg->m_originY);
			thisClient->setFrameSize(openMsg->m_originalSizeX, openMsg->m_originalSizeY);
			thisClient->PrepareImageBuffer();

			boost::shared_ptr<CqImage> baseImage = boost::static_pointer_cast<CqImage>(thisClient);
			if(window->currentBook()->framebuffer())
			{
				window->currentBook()->framebuffer()->connect(baseImage);
				window->currentBook()->framebuffer()->show();
			}
			else
			{
				Aqsis::log() << Aqsis::debug << "Creating a new FB window" << std::endl;
				boost::shared_ptr<CqFramebuffer> fb(new CqFramebuffer(thisClient->imageWidth(), thisClient->imageHeight(), thisClient->channels()));
				window->currentBook()->setFramebuffer(fb);
				fb->show();
				fb->connect(baseImage);
			}
#endif
		}
		else if(root->ValueStr().compare("Data") == 0)
		{
			//thisClient->acceptData(dataMsg);
		}
		//else if(msg.m_MessageID == MessageID_Filename)
		//{
		//	SqDDMessageFilename* fnameMsg = reinterpret_cast<SqDDMessageFilename*>(buff);
		//	thisClient->setName(std::string(fnameMsg->m_String, fnameMsg->m_StringLength));
		//	window->updateImageList(window->currentBookName());
		//}
		else if(root->ValueStr().compare("Close") == 0)
		{
			Aqsis::log() << Aqsis::debug << "Closing socket" << std::endl;
			Fl::remove_fd(sock);
			close(sock);
			thisClient->close();
		}
	}		
}

void HandleConnection(int sock, void *data)
{
	Aqsis::log() << Aqsis::debug << "Connection established with display server" << std::endl;

	boost::shared_ptr<CqDisplayServerImage> newImage(new CqDisplayServerImage());
	newImage->setName("Unnamed");
	
	if(g_theServer.Accept(newImage))
	{
		g_theClients[newImage->socket()] = newImage;
		Fl::add_fd(newImage->socket(), FL_READ, &HandleData);
		if(window)
		{
			boost::shared_ptr<CqImage> baseImage = boost::static_pointer_cast<CqImage>(newImage);
			window->addImageToCurrentBook(baseImage);
		}
	}
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
	ap.argString( "i", "\aSpecify the address to listen on (default: " + g_strInterface + ")", &g_strInterface );
	ap.argString( "p", "\aSpecify the port to listen on (default: " + g_strPort + ")", &g_strPort );
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

