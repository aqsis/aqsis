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

#include "logging.h"
#include "logging_streambufs.h"
#include "sstring.h"
#include "socket.h"
#include "image.h"

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

#ifndef	AQSIS_SYSTEM_WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#else
#include <winsock2.h>
#endif

#include "piqsl.h"
#include "fluid_piqsl_ui.h"
#include "displayserverimage.h"
#include "framebuffer.h"
#include "book.h"
#include "tinyxml.h"
#include <FL/Fl.H>


ArgParse::apstring      g_strInterface = "127.0.0.1";
ArgParse::apstring      g_strPort = "49515";
bool    		g_fHelp = 0;
bool    		g_fVersion = 0;
bool 			g_cl_no_color = false;
bool 			g_cl_syslog = false;
ArgParse::apint 	g_cl_verbose = 1;

CqPiqslMainWindow *window = 0;
boost::mutex g_XMLMutex;

std::map<std::string, TqInt>	g_mapNameToType;
std::map<TqInt, std::string>	g_mapTypeToName;

void version( std::ostream& Stream )
{
	Stream << "piqsl version " << VERSION_STR_PRINT << std::endl << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
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

CqSocket g_theSocket;
std::vector<boost::thread*> g_theThreads;
#define BUF_SIZE  4096

class CqDataHandler
{
	public:
		CqDataHandler(boost::shared_ptr<CqDisplayServerImage> thisClient) : m_client(thisClient), m_done(false)
		{}

		void operator()()
		{
			handleData();
		}

		void handleData()
		{
			std::stringstream buffer;
			int count;

			// Read a message
			while(!m_done)	
			{
				count = m_client->socket().recvData(buffer);
				if(count <= 0)
					break;
				// Readbuf should now contain a complete message
				processMessage(buffer);
				buffer.str("");
				buffer.clear();
			}
		}

		int sendXMLMessage(TiXmlDocument& msg)
		{
			std::stringstream message;
			message << msg;

			return( m_client->socket().sendData( message.str() ) );
		}

		void processMessage(std::stringstream& msg)
		{
			boost::mutex::scoped_lock lock(g_XMLMutex);
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
						param = child->FirstChildElement("FloatsParameter");
						while(param)
						{
							const char* name = param->Attribute("name");
							if(std::string("quantize").compare(name) == 0)
							{
								TqFloat quantize[4];
								TiXmlElement* values = param->FirstChildElement("Values");
								if(values)
								{
									double val;
									TiXmlElement* value = values->FirstChildElement("Float");
									value->Attribute("value", &val);
									quantize[0] = val;
									value = value->NextSiblingElement("Float");
									value->Attribute("value", &val);
									quantize[0] = val;
									value = value->NextSiblingElement("Float");
									value->Attribute("value", &val);
									quantize[0] = val;
									value = value->NextSiblingElement("Float");
									value->Attribute("value", &val);
									quantize[0] = val;
									m_client->setQuantize(quantize);
								}
							}
							param = param->NextSiblingElement("FloatsParameter");
						}
					}
					child = root->FirstChildElement("Formats");
					if(child)
					{
						//int channels = 0;
						TiXmlElement* format = child->FirstChildElement("Format");
						while(format)
						{
							//++channels;
							// Read the format type from the node.
							const char* typeName = format->GetText();
							const char* formatName = format->Attribute("name");
							TqInt typeID = PkDspyUnsigned8;
							std::map<std::string, TqInt>::iterator type;
							if((type = g_mapNameToType.find(typeName)) != g_mapNameToType.end())
								typeID = type->second;
							m_client->addChannel(formatName, typeID);

							format = format->NextSiblingElement("Format");
						}
						// Ensure that the formats are in the right order.
						m_client->reorderChannels();
						// Send the reorganised formats back.
						TiXmlDocument doc("formats.xml");
						TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
						TiXmlElement* formatsXML = new TiXmlElement("Formats");
						TqInt ichannel;
						for( ichannel = 0; ichannel < m_client->numChannels(); ++ichannel)
						{
							TiXmlElement* formatv = new TiXmlElement("Format");
							formatv->SetAttribute("name", m_client->channelName(ichannel));
							TiXmlText* formatText = new TiXmlText(g_mapTypeToName[m_client->channelType(ichannel)]);
							formatv->LinkEndChild(formatText); 
							formatsXML->LinkEndChild(formatv);
						}
						doc.LinkEndChild(decl);
						doc.LinkEndChild(formatsXML);
						sendXMLMessage(doc);
					}
					m_client->PrepareImageBuffer();

					boost::shared_ptr<CqImage> baseImage = boost::static_pointer_cast<CqImage>(m_client);
					window->currentBook()->framebuffer()->queueResize();
					Fl::awake();
					window->currentBook()->framebuffer()->update();
					window->updateImageList();
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
				{
					m_client->close();
					m_done = true;
				}
			}		
		}


	private:
		boost::shared_ptr<CqDisplayServerImage> m_client;
		bool	m_done;
};


void HandleConnection(int sock, void *data)
{
	Aqsis::log() << Aqsis::info << "Connection established with display server" << std::endl;

	boost::shared_ptr<CqDisplayServerImage> newImage(new CqDisplayServerImage());
	newImage->setName("Unnamed");
	
	if(g_theSocket.accept(newImage->socket()))
	{
		if(window)
		{
			Fl::lock();
			boost::shared_ptr<CqImage> baseImage = boost::static_pointer_cast<CqImage>(newImage);
			window->addImageToCurrentBook(baseImage);
			Fl::unlock();
		}
		
		g_theThreads.push_back(new boost::thread(CqDataHandler(newImage)));
	}
}


int main( int argc, char** argv )
{
	Fl::lock();
	// Create listening socket. 
	// Setup fltk. 
	// add the socket to the fltk event
	// run fltk
	// blah
	ArgParse ap;

	// Set up the options
	ap.usageHeader( ArgParse::apstring( "Usage: " ) + argv[ 0 ] + " [options]" );
	ap.argString( "i", "\aSpecify the IP address to listen on (default: %default)", &g_strInterface );
	ap.argString( "p", "\aSpecify the TCP port to listen on (default: %default)", &g_strPort );
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

	// Fill in the typenames maps
	g_mapNameToType["PkDspyFloat32"] = PkDspyFloat32;
	g_mapNameToType["PkDspyUnsigned32"] = PkDspyUnsigned32;
	g_mapNameToType["PkDspySigned32"] = PkDspySigned32;
	g_mapNameToType["PkDspyUnsigned16"] = PkDspyUnsigned16;
	g_mapNameToType["PkDspySigned16"] = PkDspySigned16;
	g_mapNameToType["PkDspyUnsigned8"] = PkDspyUnsigned8;
	g_mapNameToType["PkDspySigned8"] = PkDspySigned8;
	g_mapNameToType["PkDspyString"] = PkDspyString;
	g_mapNameToType["PkDspyMatrix"] = PkDspyMatrix;

	g_mapTypeToName[PkDspyFloat32] = "PkDspyFloat32";
	g_mapTypeToName[PkDspyUnsigned32] = "PkDspyUnsigned32";
	g_mapTypeToName[PkDspySigned32] = "PkDspySigned32";
	g_mapTypeToName[PkDspyUnsigned16] = "PkDspyUnsigned16";
	g_mapTypeToName[PkDspySigned16] = "PkDspySigned16";
	g_mapTypeToName[PkDspyUnsigned8] = "PkDspyUnsigned8";
	g_mapTypeToName[PkDspySigned8] = "PkDspySigned8";
	g_mapTypeToName[PkDspyString] = "PkDspyString";
	g_mapTypeToName[PkDspyMatrix] = "PkDspyMatrix";

	int portno = atoi(g_strPort.c_str());
	CqSocket::initialiseSockets();
	if(!g_theSocket.prepare(g_strInterface, portno))
		Aqsis::log() << Aqsis::error << "Cannot open server on the specified port" << std::endl;

	Fl::add_fd(g_theSocket,&HandleConnection);

	window = new CqPiqslMainWindow();
	char *internalArgs[] = {
		"piqsl"
	};
	window->show(1, internalArgs);


	int result = 0;
	for(;;)
	{
		Fl::wait();
		// Act upon an resize/update requests on the framebuffers.
		CqPiqslBase::TqBookListIterator book;
		for(book = window->booksBegin(); book != window->booksEnd(); ++book)
		{
			if((*book)->framebuffer())
				(*book)->framebuffer()->checkResize();
		}
		if(Fl::first_window() == NULL)
			break;
	}

	return(result);
}

