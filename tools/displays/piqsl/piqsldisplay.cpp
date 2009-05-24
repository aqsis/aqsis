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
		\brief A display device that communicates with a separate process
			using sockets and XML based data packets.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include <aqsis/aqsis.h>

#include <cstring>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/shared_ptr.hpp>

#ifdef AQSIS_SYSTEM_WIN32
	#include <winsock2.h>
	typedef	u_long in_addr_t;
#else
	#include <errno.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <time.h>  // for nanosleep()
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#define	INVALID_SOCKET -1
#endif
#ifdef AQSIS_SYSTEM_MACOSX
  #include <Carbon/Carbon.h>
#endif

#include <tinyxml.h>

#include <aqsis/ri/ndspy.h>
#include "dspyhlpr.h"
#include <aqsis/version.h>
#include <aqsis/util/socket.h>
#include <aqsis/util/logging.h>
#include <aqsis/util/logging_streambufs.h>
#include <aqsis/math/math.h>

using namespace Aqsis;

struct SqPiqslDisplayInstance
{
	std::string		m_filename;
	std::string		m_hostname;
	TqInt			m_port;
	CqSocket		m_socket;
	// The number of pixels that have already been rendered (used for progress reporting)
	TqInt		m_pixelsReceived;

	friend std::istream& operator >>(std::istream &is,struct SqPiqslDisplayInstance &obj);
	friend std::ostream& operator <<(std::ostream &os,const struct SqPiqslDisplayInstance &obj);
};

static int sendXMLMessage(TiXmlDocument& msg, CqSocket& sock);
static boost::shared_ptr<TiXmlDocument> recvXMLMessage(CqSocket& sock);

// Define a base64 encoding stream iterator using the boost archive data flow iterators.
typedef 
    boost::archive::iterators::insert_linebreaks<         // insert line breaks every 72 characters
        boost::archive::iterators::base64_from_binary<    // convert binary values ot base64 characters
            boost::archive::iterators::transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
                const char *,
                6,
                8
            >
        > 
        ,72
    > 
    base64_text; // compose all the above operations in to a new iterator

// Macros to initialise the type/name name/type maps.
#define	INIT_TYPE_NAME_MAPS(name) \
g_mapNameToType[#name] = name; \
g_mapTypeToName[name] = #name;

std::map<std::string, TqInt>	g_mapNameToType;
std::map<TqInt, std::string>	g_mapTypeToName;

extern "C" PtDspyError DspyImageOpen(PtDspyImageHandle * image,
                          const char *drivername,
                          const char *filename,
                          int width,
                          int height,
                          int paramCount,
                          const UserParameter *parameters,
                          int iFormatCount,
                          PtDspyDevFormat *format,
                          PtFlagStuff *flagstuff)
{
	SqPiqslDisplayInstance* pImage;

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

	pImage = new SqPiqslDisplayInstance;
	flagstuff->flags = 0;

	if(pImage)
	{
		// Store the instance information so that on re-entry we know which display is being referenced.
		*image = pImage;

		pImage->m_filename = filename;

		int scanorder;
		if( DspyFindIntInParamList("scanlineorder", &scanorder, paramCount, parameters ) == PkDspyErrorNone )
		{
			flagstuff->flags = PkDspyFlagsWantsScanLineOrder;
		}

		// We need to start a framebuffer if none is running yet
		// Need to create our actual socket

		// Check if the user has specified any options
		char *hostname = NULL;
		if( DspyFindStringInParamList("host", &hostname, paramCount, parameters ) == PkDspyErrorNone )
			pImage->m_hostname = hostname;
		else 
			pImage->m_hostname =  "127.0.0.1";

		char *port = NULL;
		if( DspyFindStringInParamList("port", &port, paramCount, parameters ) == PkDspyErrorNone )
			pImage->m_port = atoi(port);
		else 
			pImage->m_port = 49515;

		// First, see if piqsl is running, by trying to connect to it.
		CqSocket::initialiseSockets();
		pImage->m_socket.connect(pImage->m_hostname, pImage->m_port);
		if(!pImage->m_socket)
		{
			Aqsis::log() << info << "Will try to start a framebuffer" << std::endl;
			//Local FB requested, we've not run one yet
			// \todo: Need to abstract this into a system specific applciation launch in aqsistypes (pgregory).
#ifndef	AQSIS_SYSTEM_WIN32
			int pid = fork();
			if (pid != -1)
			{
				if (!pid)
				{
					// Child process executes the following after forking.
					char arg1[] = "piqsl";
					char arg2[] = "-i";
					char arg3[] = "127.0.0.1";
					char* argv[4] = {arg1, arg2, arg3, NULL};

#if defined AQSIS_SYSTEM_MACOSX
					// TODO: need to pass verbosity level for logginng
					signal(SIGHUP, SIG_IGN);
					nice(2);
					if(execvp("piqsl",argv) < 0)
					{
						CFURLRef pluginRef = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
						CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
						const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
						std::string program = pathPtr;
						program.append("/piqsl");
						argv[0] = strdup(program.c_str());
						execvp(program.c_str(), argv);
						free(argv[0]);
					}
#else
					// TODO: need to pass verbosity level for logginng
					signal(SIGHUP, SIG_IGN);
					nice(2);
					execvp("piqsl",argv);
#endif
					// The child process shouldn't end up here.  If it does
					// there's an error and we should terminate now after
					// trying to report the problem.
					switch(errno)
					{
						case EACCES:
							Aqsis::log() << error << "access denied when executing piqsl\n";
							break;
						case ENOENT:
							Aqsis::log() << error << "piqsl executable not found\n";
							break;
						default:
							Aqsis::log() << error << "Could not execute piqsl\n";
							break;
					}
					exit(1);
				}
			}
			else
			{
				// An error occurred
				Aqsis::log() << error << "Could not fork()" << std::endl;
				return(PkDspyErrorNoMemory);
			}
#else
			
			PROCESS_INFORMATION piProcInfo;
			STARTUPINFO siStartInfo;
			BOOL bFuncRetn = FALSE;

			ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
			siStartInfo.cb = sizeof(STARTUPINFO);
			ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

			// Create the child process.
			Aqsis::log() << info << "Starting the framebuffer" << std::endl;

			char *command = "piqsl -i 127.0.0.1";
			bFuncRetn = CreateProcess(NULL,
									  command,       // command line
									  NULL,          // process security attributes
									  NULL,          // primary thread security attributes
									  TRUE,          // handles are inherited
									  IDLE_PRIORITY_CLASS,             // creation flags
									  NULL,          // use parent's environment
									  NULL,          // use parent's current directory
									  &siStartInfo,  // STARTUPINFO pointer
									  &piProcInfo);  // receives PROCESS_INFORMATION

			if (bFuncRetn == 0)
			{
				Aqsis::log() << error << "RiProcRunProgram: CreateProcess failed" << std::endl;
				return(PkDspyErrorUndefined);
			}
#endif
			// The framebuffer should be running at this point, but may not be
			// responsive right away.  We try to connect at fixed intervals
			// until we get a response or pass the timeout.
			//
			// timeRemaining and interval are in milliseconds.
			long timeRemaining = 10000;
			const long interval = 100;
			while(!pImage->m_socket.connect(pImage->m_hostname, pImage->m_port) && timeRemaining > 0)
			{
#				ifdef AQSIS_SYSTEM_WIN32
					Sleep(interval);
#				else
					timespec sleepTime;
					sleepTime.tv_sec = 0;
					sleepTime.tv_nsec = 1000000*interval;
					nanosleep(&sleepTime, 0);
#				endif
				timeRemaining -= interval;
			}
		}
		if(pImage->m_socket)
		{
			TiXmlDocument displaydoc("open.xml");
			TiXmlDeclaration* displaydecl = new TiXmlDeclaration("1.0","","yes");
			TiXmlElement* openMsgXML = new TiXmlElement("Open");

			TiXmlElement* nameXML = new TiXmlElement("Name");
			TiXmlText* nameText = new TiXmlText(filename);
			nameXML->LinkEndChild(nameText);
			openMsgXML->LinkEndChild(nameXML);

			TiXmlElement* typeXML = new TiXmlElement("Type");
			TiXmlText* typeText = new TiXmlText(drivername);
			typeXML->LinkEndChild(typeText);
			openMsgXML->LinkEndChild(typeXML);

			TiXmlElement* dimensionsXML = new TiXmlElement("Dimensions");
			dimensionsXML->SetAttribute("width", width);
			dimensionsXML->SetAttribute("height", height);
			openMsgXML->LinkEndChild(dimensionsXML);

			TiXmlElement* paramsXML = new TiXmlElement("Parameters");
			int iparam;
			for(iparam = 0; iparam < paramCount; iparam++)
			{
				switch(parameters[iparam].vtype)
				{
					case 'i':
					{
						TiXmlElement* param = new TiXmlElement("IntsParameter");
						param->SetAttribute("name", parameters[iparam].name);
						paramsXML->LinkEndChild(param);
						int ivalue;
						int* pvalues = reinterpret_cast<int*>(parameters[iparam].value);
						TiXmlElement* values = new TiXmlElement("Values");
						for(ivalue = 0; ivalue < parameters[iparam].vcount; ++ivalue)
						{
							TiXmlElement* value = new TiXmlElement("Int");
							value->SetAttribute("value", pvalues[ivalue]);
							values->LinkEndChild(value);
						}
						param->LinkEndChild(values);
						break;
					}

					case 'f':
					{
						TiXmlElement* param = new TiXmlElement("FloatsParameter");
						param->SetAttribute("name", parameters[iparam].name);
						paramsXML->LinkEndChild(param);
						int ivalue;
						float* pvalues = reinterpret_cast<float*>(parameters[iparam].value);
						TiXmlElement* values = new TiXmlElement("Values");
						for(ivalue = 0; ivalue < parameters[iparam].vcount; ++ivalue)
						{
							TiXmlElement* value = new TiXmlElement("Float");
							value->SetDoubleAttribute("value", static_cast<double>(pvalues[ivalue]));
							values->LinkEndChild(value);
						}
						param->LinkEndChild(values);
						break;
					}

					case 's':
					{
						TiXmlElement* param = new TiXmlElement("StringsParameter");
						param->SetAttribute("name", parameters[iparam].name);
						paramsXML->LinkEndChild(param);
						int ivalue;
						char** pvalues = reinterpret_cast<char**>(parameters[iparam].value);
						TiXmlElement* values = new TiXmlElement("Values");
						for(ivalue = 0; ivalue < parameters[iparam].vcount; ++ivalue)
						{
							TiXmlElement* value = new TiXmlElement("String");
							TiXmlText* valueText = new TiXmlText(pvalues[ivalue]);
							value->LinkEndChild(valueText); 
							values->LinkEndChild(value);
						}
						param->LinkEndChild(values);
						break;
					}
				}
			}
			openMsgXML->LinkEndChild(paramsXML);

			TiXmlElement* formatsXML = new TiXmlElement("Formats");
			int iformat;
			for(iformat = 0; iformat < iFormatCount; ++iformat)
			{
				TiXmlElement* formatv = new TiXmlElement("Format");
				formatv->SetAttribute("name", format[iformat].name);
				TiXmlText* formatText = new TiXmlText(g_mapTypeToName[
						PkDspyMaskType & format[iformat].type]);
				formatv->LinkEndChild(formatText); 
				formatsXML->LinkEndChild(formatv);
			}
			openMsgXML->LinkEndChild(formatsXML);
			displaydoc.LinkEndChild(displaydecl);
			displaydoc.LinkEndChild(openMsgXML);
			sendXMLMessage(displaydoc, pImage->m_socket);
			boost::shared_ptr<TiXmlDocument> formats = recvXMLMessage(pImage->m_socket);
			TiXmlElement* child = formats->FirstChildElement("Formats");
			if(child)
			{
				TiXmlElement* formatNode = child->FirstChildElement("Format");
				// If we are recieving "rgba" data, ensure that it is in the
				// correct order.  First copy the XML "formats" document into
				// an array, outFormat:
				std::vector<PtDspyDevFormat> outFormat;
				outFormat.reserve(iFormatCount);
				TqInt iformat = 0;
				while(formatNode)
				{
					// Read the format type from the node.
					const char* typeName = formatNode->GetText();
					const char* formatName = formatNode->Attribute("name");
					TqInt typeID = g_mapNameToType[typeName];
					char* name = new char[strlen(formatName)+1];
					strcpy(name, formatName);
					PtDspyDevFormat fmt = {name, typeID};
					outFormat.push_back(fmt);
					formatNode = formatNode->NextSiblingElement("Format");
					iformat++;
				}
				// Now reorder the output parameter array "format" using
				// outFormats as a reference for the desired order.
				PtDspyError err = DspyReorderFormatting(iFormatCount, format,
						Aqsis::min(iFormatCount,4), &outFormat[0]);
				for(TqInt i = 0, end = outFormat.size(); i < end; ++i)
					delete[] outFormat[i].name;
				if( err != PkDspyErrorNone )
				{
					return(err);
				}
			}
		}
		else 
			return(PkDspyErrorUndefined);
	} 
	else 
		return(PkDspyErrorUndefined);

	return(PkDspyErrorNone);
}


extern "C" PtDspyError DspyImageData(PtDspyImageHandle image,
                          int xmin,
                          int xmaxplus1,
                          int ymin,
                          int ymaxplus1,
                          int entrysize,
                          const unsigned char *data)
{
	SqPiqslDisplayInstance* pImage;
	pImage = reinterpret_cast<SqPiqslDisplayInstance*>(image);

	TqInt bucketlinelen = entrysize * (xmaxplus1 - xmin);
	TqInt bufferlength = bucketlinelen * (ymaxplus1 - ymin);
	TiXmlDocument msg;
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "yes");
	TiXmlElement* dataMsgXML = new TiXmlElement("Data");
	TiXmlElement* dimensionsXML = new TiXmlElement("Dimensions");
	dimensionsXML->SetAttribute("xmin", xmin);
	dimensionsXML->SetAttribute("xmaxplus1", xmaxplus1);
	dimensionsXML->SetAttribute("ymin", ymin);
	dimensionsXML->SetAttribute("ymaxplus1", ymaxplus1);
	dimensionsXML->SetAttribute("elementsize", entrysize);
	dataMsgXML->LinkEndChild(dimensionsXML);

	TiXmlElement* bucketDataXML = new TiXmlElement("BucketData");
	std::stringstream base64Data;
	std::copy(	base64_text(BOOST_MAKE_PFTO_WRAPPER(data)), 
				base64_text(BOOST_MAKE_PFTO_WRAPPER(data + bufferlength)), 
				std::ostream_iterator<char>(base64Data));
	TiXmlText* dataTextXML = new TiXmlText(base64Data.str());
	dataTextXML->SetCDATA(true);
	bucketDataXML->LinkEndChild(dataTextXML);
	dataMsgXML->LinkEndChild(bucketDataXML);

	msg.LinkEndChild(decl);
	msg.LinkEndChild(dataMsgXML);
	sendXMLMessage(msg, pImage->m_socket);

	return(PkDspyErrorNone);
}


extern "C" PtDspyError DspyImageClose(PtDspyImageHandle image)
{
	/// \note: This should never be called, as Aqsis will look 
	///		  for the DspyImageDelayClose first and use that if it can.
	///		  but just in case, it's important we get an ack from
	///		  piqsl, so we forward the call.
	return DspyImageDelayClose(image);
}


extern "C" PtDspyError DspyImageDelayClose(PtDspyImageHandle image)
{
	SqPiqslDisplayInstance* pImage;
	pImage = reinterpret_cast<SqPiqslDisplayInstance*>(image);
	
	// Close the socket
	if(pImage && pImage->m_socket)
	{
		TiXmlDocument doc("close.xml");
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
		TiXmlElement* closeMsgXML = new TiXmlElement("Close");
		doc.LinkEndChild(decl);
		doc.LinkEndChild(closeMsgXML);
		sendXMLMessage(doc, pImage->m_socket);
		recvXMLMessage(pImage->m_socket);
	}
	delete pImage;

	return(PkDspyErrorNone);
}


extern "C" PtDspyError DspyImageQuery(PtDspyImageHandle image,
                           PtDspyQueryType type,
                           size_t size,
                           void *data)
{
	SqPiqslDisplayInstance* pImage;
	pImage = reinterpret_cast<SqPiqslDisplayInstance*>(image);

	//PtDspyOverwriteInfo overwriteInfo;
	//PtDspySizeInfo sizeInfo;

	if(size <= 0 || !data)
		return PkDspyErrorBadParams;

#if 0
	switch (type)
	{
			case PkOverwriteQuery:
			{
				if ((TqUint) size > sizeof(overwriteInfo))
					size = sizeof(overwriteInfo);
				overwriteInfo.overwrite = 1;
				overwriteInfo.interactive = 0;
				memcpy(data, &overwriteInfo, size);
			}
			break;
			case PkSizeQuery:
			{
				if ((TqUint) size > sizeof(sizeInfo))
					size = sizeof(sizeInfo);
				if(pImage)
				{
					if(!pImage->m_width || !pImage->m_height)
					{
						pImage->m_width = 640;
						pImage->m_height = 480;
					}
					sizeInfo.width = pImage->m_width;
					sizeInfo.height = pImage->m_height;
					sizeInfo.aspectRatio = 1.0f;
				}
				else
				{
					sizeInfo.width = 640;
					sizeInfo.height = 480;
					sizeInfo.aspectRatio = 1.0f;
				}
				memcpy(data, &sizeInfo, size);
			}
			break;
			default:
			return PkDspyErrorUnsupported;
	}
#endif

	return(PkDspyErrorNone);
}

static int sendXMLMessage(TiXmlDocument& msg, CqSocket& sock)
{
	std::stringstream message;
	message << msg;

	return( sock.sendData( message.str() ) );
}

static boost::shared_ptr<TiXmlDocument> recvXMLMessage(CqSocket& sock)
{
	boost::shared_ptr<TiXmlDocument> xmlMsg(new TiXmlDocument());
	std::stringstream buffer;
	int len = sock.recvData(buffer);
	if(len > 0)
	{
		// Parse the XML message sent.
		xmlMsg->Parse(buffer.str().c_str());
	}
	return(xmlMsg);
}


