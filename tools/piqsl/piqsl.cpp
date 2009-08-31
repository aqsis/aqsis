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
#include	<FL/Fl.H>
#include	<tinyxml.h>

#include	<aqsis/util/argparse.h>
#include	<aqsis/util/logging.h>
#include	<aqsis/util/logging_streambufs.h>
#include	<aqsis/util/sstring.h>
#include	<aqsis/util/socket.h>
#include	<aqsis/version.h>

#include	"piqsl.h"
#include	"displayserverimage.h"
#include	"book.h"
#include	"piqslmainwindow.h"
#include	"image.h"

using namespace Aqsis;

ArgParse::apstring      g_strInterface = "127.0.0.1";
ArgParse::apstring      g_strPort = "49515";
bool    		g_fHelp = 0;
bool    		g_fVersion = 0;
bool 			g_cl_no_color = false;
bool 			g_cl_syslog = false;
ArgParse::apint 	g_cl_verbose = 1;

CqPiqslMainWindow* window = 0;
boost::mutex g_XMLMutex;

std::map<std::string, TqInt>	g_mapNameToType;
std::map<TqInt, std::string>	g_mapTypeToName;

void version( std::ostream& Stream )
{
	Stream << "piqsl version " << AQSIS_VERSION_STR_FULL << std::endl << "compiled " << __DATE__ << " " << __TIME__ << std::endl;
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
						if (fname == NULL)
						{
							fname = "ri.pic";
						}
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
						double clipNear = 0;
						double clipFar = FLT_MAX;
						while(param)
						{
							const char* name = param->Attribute("name");
							if(name == std::string("near"))
							{
								TiXmlElement* value = param->FirstChildElement("Values")
									->FirstChildElement("Float");
								value->Attribute("value", &clipNear);
							}
							else if(name == std::string("far"))
							{
								TiXmlElement* value = param->FirstChildElement("Values")
									->FirstChildElement("Float");
								value->Attribute("value", &clipFar);
							}
							param = param->NextSiblingElement("FloatsParameter");
						}
						m_client->setClipping(clipNear, clipFar);
					}
					child = root->FirstChildElement("Formats");
					CqChannelList channelList;
					if(child)
					{
						TiXmlElement* format = child->FirstChildElement("Format");
						while(format)
						{
							// Read the format type from the node.
							const char* typeName = format->GetText();
							const char* formatName = format->Attribute("name");
							TqInt typeID = PkDspyUnsigned8;
							std::map<std::string, TqInt>::iterator type;
							if((type = g_mapNameToType.find(typeName)) != g_mapNameToType.end())
								typeID = type->second;
							channelList.addChannel(SqChannelInfo(formatName, chanFormatFromPkDspy(typeID)));

							format = format->NextSiblingElement("Format");
						}
						// Ensure that the formats are in the right order.
						channelList.reorderChannels();
						// Send the reorganised formats back.
						TiXmlDocument doc("formats.xml");
						TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
						TiXmlElement* formatsXML = new TiXmlElement("Formats");
						for(CqChannelList::const_iterator ichan = channelList.begin();
								ichan != channelList.end(); ++ichan)
						{
							TiXmlElement* formatv = new TiXmlElement("Format");
							formatv->SetAttribute("name", ichan->name);
							TiXmlText* formatText = new TiXmlText(g_mapTypeToName[pkDspyFromChanFormat(ichan->type)]);
							formatv->LinkEndChild(formatText); 
							formatsXML->LinkEndChild(formatv);
						}
						doc.LinkEndChild(decl);
						doc.LinkEndChild(formatsXML);
						sendXMLMessage(doc);
					}
					m_client->prepareImageBuffers(channelList);

					window->queueResize();
					Fl::awake();
//					window->update();
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
					// Send and acknowledge.
					TiXmlDocument doc("ack.xml");
					TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
					TiXmlElement* formatsXML = new TiXmlElement("Acknowledge");
					doc.LinkEndChild(decl);
					doc.LinkEndChild(formatsXML);
					sendXMLMessage(doc);
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

// Macros to initialise the type/name name/type maps.
#	define	INIT_TYPE_NAME_MAPS(name) \
	g_mapNameToType[#name] = name; \
	g_mapTypeToName[name] = #name;

	// Fill in the typenames maps
	INIT_TYPE_NAME_MAPS(PkDspyFloat32);
	INIT_TYPE_NAME_MAPS(PkDspyUnsigned32);
	INIT_TYPE_NAME_MAPS(PkDspySigned32);
	INIT_TYPE_NAME_MAPS(PkDspyUnsigned16);
	INIT_TYPE_NAME_MAPS(PkDspySigned16);
	INIT_TYPE_NAME_MAPS(PkDspyUnsigned8);
	INIT_TYPE_NAME_MAPS(PkDspySigned8);
	INIT_TYPE_NAME_MAPS(PkDspyString);
	INIT_TYPE_NAME_MAPS(PkDspyMatrix);

#	undef INIT_TYPE_NAME_MAPS

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


