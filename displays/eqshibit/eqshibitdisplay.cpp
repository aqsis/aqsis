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

#include <iostream>
#include <logging.h>
#include <logging_streambufs.h>
#include "sstring.h"

using namespace Aqsis;

#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <float.h>
#include <time.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ndspy.h"

#include "version.h"

#include "eqshibitdisplay.h"
#include "displaydriver.h"
#include "tinyxml.h"

// From displayhelpers.c
#ifdef __cplusplus
extern "C"
{
#endif
	PtDspyError DspyReorderFormatting(int formatCount, PtDspyDevFormat *format, int outFormatCount, const PtDspyDevFormat *outFormat);
	PtDspyError DspyFindStringInParamList(const char *string, char **result, int n, const UserParameter *p);
	PtDspyError DspyFindIntInParamList(const char *string, int *result, int n, const UserParameter *p);
	PtDspyError DspyFindFloatInParamList(const char *string, float *result, int n, const UserParameter *p);
	PtDspyError DspyFindMatrixInParamList(const char *string, float *result, int n, const UserParameter *p);
	PtDspyError DspyFindIntsInParamList(const char *string, int *resultCount, int *result, int n, const UserParameter *p);
#ifdef __cplusplus
}
#endif

static int sendXMLMessage(TiXmlDocument& msg, int sock);

PtDspyError DspyImageOpen(PtDspyImageHandle * image,
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
	SqDisplayInstance* pImage;

	pImage = new SqDisplayInstance;
	flagstuff->flags = 0;

	if(pImage)
	{
		// Store the instance information so that on re-entry we know which display is being referenced.
		*image = pImage;

		pImage->m_filename = new char[strlen(filename)+1];
		strcpy(pImage->m_filename, filename);

		// Scan the formats table to see what the widest channel format specified is.
		TqUint widestFormat = PkDspySigned8;
		TqInt i;
		for(i=0; i<iFormatCount; i++)
			if(format[i].type < widestFormat)
				widestFormat = format[i].type;

		if(widestFormat == PkDspySigned8)
			widestFormat = PkDspyUnsigned8;
		else if(widestFormat == PkDspySigned16)
			widestFormat = PkDspyUnsigned16;
		else if(widestFormat == PkDspySigned32)
			widestFormat = PkDspyUnsigned32;

		// If we are recieving "rgba" data, ensure that it is in the correct order.
		PtDspyDevFormat outFormat[] =
		    {
			{"r", widestFormat},
			{"g", widestFormat},
			{"b", widestFormat},
			{"a", widestFormat},
		    };
		PtDspyError err = DspyReorderFormatting(iFormatCount, format, MIN(iFormatCount,4), outFormat);
		if( err != PkDspyErrorNone )
		{
			return(err);
		}

		// We need to start a framebuffer if none is running yet
		// Need to create our actual socket

		// Check if the user has specified any options
		char *hostname = NULL;
		char *hostport = NULL;

		if( DspyFindStringInParamList("hostcomputer", &hostname, paramCount, parameters ) == PkDspyErrorNone )
		{
			Aqsis::log() << debug << "FB host specified: " << pImage->m_hostname << std::endl;
			pImage->m_hostname = strdup(hostname);
		} 
		else 
		{
			Aqsis::log() << debug << "FB not host specified" << std::endl;
			pImage->m_hostname =  NULL;
		};
		if( DspyFindStringInParamList("hostport", &hostport, paramCount, parameters ) == PkDspyErrorNone )
		{
			Aqsis::log() << debug << "FB port specified" << pImage->m_hostport << std::endl;
			pImage->m_hostport = atoi(strdup(hostport));
		} 
		else 
		{
			Aqsis::log() << debug << "FB port not specified" << std::endl;
			pImage->m_hostport = 0;
		};

		if( pImage->m_hostname == NULL  )
		{
			//No host specified
			pImage->m_hostname = "127.0.0.1";
			if( !pImage->m_hostport )  
			{
				pImage->m_hostport = atoi("48515");
			};
			Aqsis::log() << info << "Will try to start a framebuffer" << std::endl;
			//Local FB requested, we've not run one yet
			// XXX Need to Abstract this for windows to work
			int pid = fork();
			if (pid != -1)
			{
				if (pid)
				{
					Aqsis::log() << info << "Starting the framebuffer" << std::endl;
					sleep(2); //Give it time to startup
				}
				else
				{
					// TODO: need to pass verbosity level for logginng
					char *argv[4] = {"eqshibit","-i","127.0.0.1",NULL};
					signal(SIGHUP, SIG_IGN);
					execvp("eqshibit",argv);
				}
			} 
			else
			{
				// An error occurred
				Aqsis::log() << error << "Could not fork()" << std::endl;
				return(PkDspyErrorNoMemory);
			}

			// The FB should be running at this point.
			// Lets try and connect

			int sock = socket(AF_INET,SOCK_STREAM,0);
			sockaddr_in adServer;

			bzero((char *) &adServer, sizeof(adServer)); 
			adServer.sin_family = AF_INET;
			adServer.sin_addr.s_addr = inet_addr(pImage->m_hostname);
			if (adServer.sin_addr.s_addr == (in_addr_t) -1)
			{
				Aqsis::log() << error << "Invalid IP address" << std::endl;;
				return(PkDspyErrorNoMemory);
			};
			adServer.sin_port = htons(pImage->m_hostport);

			if( connect(sock,(const struct sockaddr*) &adServer, sizeof(sockaddr_in)))
			{
				return(PkDspyErrorUndefined);
			}
			pImage->m_socket = sock;

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
							value->SetAttribute("value", pvalues[ivalue]);
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
				formatsXML->LinkEndChild(formatv);
			}
			openMsgXML->LinkEndChild(formatsXML);

			displaydoc.LinkEndChild(displaydecl);
			displaydoc.LinkEndChild(openMsgXML);
			sendXMLMessage(displaydoc, pImage->m_socket);
		}
	} else 
		return(PkDspyErrorNoMemory);

	return(PkDspyErrorNone);
}


PtDspyError DspyImageData(PtDspyImageHandle image,
                          int xmin,
                          int xmaxplus1,
                          int ymin,
                          int ymaxplus1,
                          int entrysize,
                          const unsigned char *data)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

	TqInt bucketlinelen = entrysize * (xmaxplus1 - xmin);
	SqDDMessageData* msg = SqDDMessageData::Construct(xmin, xmaxplus1, ymin, ymaxplus1, entrysize, data, bucketlinelen * (ymaxplus1 - ymin));
	//msg->Send(pImage->m_socket);
	msg->Destroy();

	return(PkDspyErrorNone);
}


PtDspyError DspyImageClose(PtDspyImageHandle image)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

	// Close the socket
	if(pImage && pImage->m_socket != INVALID_SOCKET)
	{
		TiXmlDocument doc("close.xml");
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
		TiXmlElement* closeMsgXML = new TiXmlElement("Close");
		doc.LinkEndChild(decl);
		doc.LinkEndChild(closeMsgXML);
		sendXMLMessage(doc, pImage->m_socket);
	}

	// Delete the image structure.
 	if (pImage->m_hostname)
		free(pImage->m_hostname);
	delete[](pImage->m_filename);
	delete(pImage);


	return(PkDspyErrorNone);
}


PtDspyError DspyImageDelayClose(PtDspyImageHandle image)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);
	
	// Close the socket
	if(pImage && pImage->m_socket != INVALID_SOCKET)
	{
		TiXmlDocument doc("close.xml");
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0","","yes");
		TiXmlElement* closeMsgXML = new TiXmlElement("Close");
		doc.LinkEndChild(decl);
		doc.LinkEndChild(closeMsgXML);
		sendXMLMessage(doc, pImage->m_socket);
	}

	if(pImage)
	{
		// Clean up
		return(DspyImageClose(image));
	}
	return(PkDspyErrorNone);
}


PtDspyError DspyImageQuery(PtDspyImageHandle image,
                           PtDspyQueryType type,
                           int size,
                           void *data)
{
	SqDisplayInstance* pImage;
	pImage = reinterpret_cast<SqDisplayInstance*>(image);

	PtDspyOverwriteInfo overwriteInfo;
	PtDspySizeInfo sizeInfo;

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

static int sendXMLMessage(TiXmlDocument& msg, int sock)
{
	std::stringstream message;
	message << msg;
	TqInt tot = 0, need = message.str().length();
	while ( need > 0 )
	{
		TqInt n = send( sock, message.str().c_str() + tot, need, 0 );
		need -= n;
		tot += n;
	}
	return( tot );
}
