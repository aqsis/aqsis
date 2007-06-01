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

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>

#include "boost/pfto.hpp"

#include <version.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "eqshibit.h"
#include "fluid_eqshibit_ui.h"
#include "ddserver.h"
#include "displayserverimage.h"
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


typedef 
	boost::archive::iterators::transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
		boost::archive::iterators::binary_from_base64<    // convert binary values ot base64 characters
			boost::archive::iterators::remove_whitespace<
				std::string::const_iterator
			>
		>,
		8,
		6
	> 
base64_binary; // compose all the above operations in to a new iterator

CqDDServer g_theServer;
std::map<int, boost::shared_ptr<CqDisplayServerImage> >	g_theClients;
std::vector<boost::thread*> g_theThreads;
#define BUF_SIZE  4096

class CqDataHandler
{
	public:
		CqDataHandler(int sock, boost::shared_ptr<CqDisplayServerImage> thisClient) : m_sock(sock), m_client(thisClient)
		{}

		void operator()()
		{
			handleData();
		}

		void handleData()
		{
			char c;
			int count;
			std::stringstream buffer;

			// Read a message
			while(1)	
			{
				// Read some more into the buffer
				count = recv(m_sock,&c,sizeof(char),0);
				if(count <= 0)
				{
					if(errno == EAGAIN)
						continue;
					else
						break;
				}

				if(c == '\0')
				{
					// Readbuf should now contain a complete message
					processMessage(buffer);
					buffer.str("");
					buffer.clear();
				} 
				else 
				{
					buffer.put(c);
				}	
			}
		}

		void processMessage(std::stringstream& msg)
		{
			// Parse the XML message sent.
			TiXmlDocument xmlMsg;
			xmlMsg.Parse(msg.str().c_str());
			// Get the root element, which is the base type of the message.
			TiXmlElement* root = xmlMsg.RootElement();

			if(root)
			{
				// Process the message based on its type.
				if(root->ValueStr().compare("Open") == 0)
				{
					TiXmlElement* child = root->FirstChildElement("Dimensions");
					if(child)
					{
						int xres = 640, yres = 480;
						child->Attribute("width", &xres);
						child->Attribute("height", &yres);
						m_client->setImageSize(xres, yres);
					}
					
					child = root->FirstChildElement("Name");
					if(child)
					{
						const char* fname = child->GetText();
						m_client->setName(fname);
					}
					// Process the parameters
					child = root->FirstChildElement("Parameters");
					if(child)
					{
						TiXmlElement* param = child->FirstChildElement("IntsParameter");
						while(param)
						{
							const char* name = param->Attribute("name");
							if(std::string("origin").compare(name) == 0)
							{
								int origin[2];
								TiXmlElement* values = param->FirstChildElement("Values");
								if(values)
								{
									TiXmlElement* value = values->FirstChildElement("Int");
									value->Attribute("value", &origin[0]);
									value = value->NextSiblingElement("Int");
									value->Attribute("value", &origin[1]);
									m_client->setOrigin(origin[0], origin[1]);
								}
							}
							else if(std::string("OriginalSize").compare(name) == 0)
							{
								TiXmlElement* values = param->FirstChildElement("Values");
								if(values)
								{
									int OriginalSize[2];
									TiXmlElement* value = values->FirstChildElement("Int");
									value->Attribute("value", &OriginalSize[0]);
									value = value->NextSiblingElement("Int");
									value->Attribute("value", &OriginalSize[1]);
									m_client->setFrameSize(OriginalSize[0], OriginalSize[1]);
								}
							}
							param = param->NextSiblingElement("IntsParameter");
						}
					}
					child = root->FirstChildElement("Formats");
					if(child)
					{
						int channels = 0;
						TiXmlElement* format = child->FirstChildElement("Format");
						while(format)
						{
							++channels;
							format = format->NextSiblingElement("Format");
						}
						m_client->setChannels(channels);
					}
					m_client->PrepareImageBuffer();

					boost::shared_ptr<CqImage> baseImage = boost::static_pointer_cast<CqImage>(m_client);
					if(window->currentBook()->framebuffer())
					{
						window->currentBook()->framebuffer()->connect(baseImage);
						window->currentBook()->framebuffer()->show();
					}
					else
					{
						boost::shared_ptr<CqFramebuffer> fb(new CqFramebuffer(m_client->imageWidth(), m_client->imageHeight(), m_client->channels()));
						window->currentBook()->setFramebuffer(fb);
						fb->show();
						fb->connect(baseImage);
					}
					window->updateImageList(window->currentBookName());
				}
				else if(root->ValueStr().compare("Data") == 0)
				{
					TiXmlElement* dimensionsXML = root->FirstChildElement("Dimensions");
					if(dimensionsXML)
					{
						int xmin, ymin, xmaxplus1, ymaxplus1, elementSize;
						dimensionsXML->Attribute("xmin", &xmin);
						dimensionsXML->Attribute("ymin", &ymin);
						dimensionsXML->Attribute("xmaxplus1", &xmaxplus1);
						dimensionsXML->Attribute("ymaxplus1", &ymaxplus1);
						dimensionsXML->Attribute("elementsize", &elementSize);
					
						TiXmlElement* bucketDataXML = root->FirstChildElement("BucketData");
						if(bucketDataXML)
						{
							TiXmlText* dataText = static_cast<TiXmlText*>(bucketDataXML->FirstChild());
							if(dataText)
							{
								int bucketlinelen = elementSize * (xmaxplus1 - xmin);
								int count = bucketlinelen * (ymaxplus1 - ymin);
								std::string data = dataText->Value();
								std::vector<unsigned char> binaryData;
								binaryData.reserve(count);
								base64_binary ti_begin = base64_binary(BOOST_MAKE_PFTO_WRAPPER(data.begin())); 
								std::size_t padding = 2 - count % 3;
								while(--count > 0)
								{
									binaryData.push_back(static_cast<char>(*ti_begin));
									++ti_begin;
								}
								binaryData.push_back(static_cast<char>(*ti_begin));
								if(padding > 1)
									++ti_begin;
								if(padding > 2)
									++ti_begin;
								m_client->acceptData(xmin, xmaxplus1, ymin, ymaxplus1, elementSize, &binaryData[0]);
							}
						}
					}
				}
				else if(root->ValueStr().compare("Close") == 0)
					m_client->close();
			}		
		}


	private:
		int m_sock;
		boost::shared_ptr<CqDisplayServerImage> m_client;
};


void HandleConnection(int sock, void *data)
{
	Aqsis::log() << Aqsis::debug << "Connection established with display server" << std::endl;

	boost::shared_ptr<CqDisplayServerImage> newImage(new CqDisplayServerImage());
	newImage->setName("Unnamed");
	
	if(g_theServer.Accept(newImage))
	{
		g_theClients[newImage->socket()] = newImage;
		// Set socket as non-blocking
		int oldflags;
		oldflags = fcntl(sock, F_GETFL, 0);
		fcntl(sock, F_SETFL, oldflags | O_NONBLOCK);
		
		g_theThreads.push_back(new boost::thread(CqDataHandler(newImage->socket(), newImage)));

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
	ap.argString( "i", "\aSpecify the IP address to listen on (default: " + g_strInterface + ")", &g_strInterface );
	ap.argString( "p", "\aSpecify the TCP port to listen on (default: " + g_strPort + ")", &g_strPort );
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

