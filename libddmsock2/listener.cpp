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
		\brief Display device request listener.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"

#include	<iostream>
#include	<fstream>
#include	<map>
#include	<sstream>
#include	"signal.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<malloc.h>

#ifdef AQSIS_SYSTEM_WIN32

#include	<process.h>

#else // AQSIS_SYSTEM_WIN32

#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>

static const int INVALID_SOCKET = -1;
static const int SD_BOTH = 2;
static const int SOCKET_ERROR = -1;

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr* PSOCKADDR;

#endif // !AQSIS_SYSTEM_WIN32

#include	"renderer.h"
#include	"imagebuffer.h"
#include	"logging.h"
#include	"listener.h"
#include	"ddmsock2.h"
#include	"displaydriver.h"
#include	<boost/bind.hpp>
#include	"tinyxml.h"

#include	<boost/ref.hpp>

START_NAMESPACE( Aqsis )


int b64_encode(char *dest, const char *src, int len);

//---------------------------------------------------------------------
/** Default constructor.
 */

CqDisplayListener::CqDisplayListener() :	m_Socket( INVALID_SOCKET ), m_listenerThread(0)
{
}


//---------------------------------------------------------------------
/** Destructor, close all connected client sockets.
 */

CqDisplayListener::~CqDisplayListener()
{
    Close();
}

//---------------------------------------------------------------------
/** Close the servers socket, waiting for any connection activity to finish first,
 */

void CqDisplayListener::Close()
{
	m_senderThreads.join_all();
	m_listenerThread->join();
	delete(m_listenerThread);

#ifdef AQSIS_SYSTEM_WIN32
    int x = 1;
    setsockopt( m_Socket, SOL_SOCKET, SO_DONTLINGER, reinterpret_cast<const char*>( &x ), sizeof( x ) );
    shutdown( m_Socket, SD_BOTH );
    closesocket( m_Socket );
#else // AQSIS_SYSTEM_WIN32
    shutdown( m_Socket, SD_BOTH );
    close( m_Socket );
#endif // !AQSIS_SYSTEM_WIN32

    m_Socket = INVALID_SOCKET;

}


//---------------------------------------------------------------------
/** Prepare the socket to accept client connections.
 * \param port Integer port number to use.
 */

TqBool	CqDisplayListener::Prepare( TqInt port, CqDDManager* manager )
{
    m_Socket = socket( AF_INET, SOCK_STREAM, 0 );
	m_pManager = manager;

    if ( m_Socket == INVALID_SOCKET )
    {
#ifdef AQSIS_SYSTEM_WIN32
        TqInt err = WSAGetLastError();
#endif // AQSIS_SYSTEM_WIN32
        std::cerr << error << "Error opening DD server socket" << std::endl;
        return ( TqFalse );
    }

    TqInt x = 1;
    setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>( &x ), sizeof( x ) );

#ifdef AQSIS_SYSTEM_WIN32
    BOOL Ret = SetHandleInformation( ( HANDLE ) m_Socket, HANDLE_FLAG_INHERIT, 0 );
#endif // AQSIS_SYSTEM_WIN32


    SOCKADDR_IN saTemp;
    memset( &saTemp, 0, sizeof( saTemp ) );
    saTemp.sin_family = AF_INET;
    saTemp.sin_port = htons( port );
    saTemp.sin_addr.s_addr = INADDR_ANY;

    while ( bind( m_Socket, ( PSOCKADDR ) & saTemp, sizeof( saTemp ) ) == SOCKET_ERROR )
    {
#ifdef AQSIS_SYSTEM_WIN32
        TqInt iE = WSAGetLastError();
        if ( errno == WSAEADDRINUSE )
#else // AQSIS_SYSTEM_WIN32
        if ( errno == EADDRINUSE )
#endif // AQSIS_SYSTEM_WIN32
        {
            port++;
            saTemp.sin_port = htons( port );
            continue;
        }
        else
        {
            std::cerr << error << "Error binding to DD socket" << std::endl;
            Close();
            return ( TqFalse );
        }
    }
    m_Port = port;

    if ( listen( m_Socket, SOMAXCONN ) == SOCKET_ERROR )
    {
        std::cerr << error << "Error listening to DD socket" << std::endl;
        Close();
        return ( TqFalse );
    }
	
	m_listenerThread = new boost::thread(boost::ref(*this));
	
	return ( TqTrue );
}


//---------------------------------------------------------------------
/** Set up the thread to wait for client connection requests.
 */

void CqDisplayListener::operator()()
{
    SOCKET c;

//	std::cerr << debug << "Listener thread started: " << reinterpret_cast<long>(this) << std::endl;

    while(1)
	{
		c = accept( m_Socket, NULL, NULL );
		if(c != INVALID_SOCKET)
		{
			m_senderThreads.add_thread(new boost::thread(CqSender(c, m_pManager)));
		}
	}
//	std::cerr << debug << "Listener thread ended: " << reinterpret_cast<long>(this) << std::endl;
}


CqSender::CqSender(int socket, CqDDManager* manager)
{
	m_Socket = socket;
	m_pManager = manager;
}

//---------------------------------------------------------------------
/** Set up the thread to wait for client connection requests.
 */

void CqSender::operator()()
{
//	std::cerr << debug << "Sender thread started: " << reinterpret_cast<long>(this) << std::endl;
	char req[255];
	int n;
	while(( n = recv( m_Socket, req, 255, 0 ) ) != 0 )
	{
		req[n] = '\0';

		// Parse the XML message with TinyXML
		TiXmlDocument request;
		request.Parse(req);

		TiXmlHandle reqHandle(&request);
		TiXmlElement* reqElement;
		if((reqElement = reqHandle.FirstChild("aqsis:request").FirstChild("aqsis:format").Element()) != NULL)
		{
			std::string desc;
			std::vector<TqInt> counts;
			TqInt datasize = QGetRenderContext()->GetOutputDataInfo(desc,counts);
			// Construct a response
			TiXmlDocument doc;
			TiXmlElement root("aqsis:response");
			TiXmlElement format("aqsis:format");
			format.SetAttribute("bucketsperrow", ToString(QGetRenderContext()->pImage()->cXBuckets()).c_str());
			format.SetAttribute("bucketspercol", ToString(QGetRenderContext()->pImage()->cYBuckets()).c_str());
			format.SetAttribute("xres", ToString(QGetRenderContext()->pImage()->iXRes()).c_str());
			format.SetAttribute("yres", ToString(QGetRenderContext()->pImage()->iYRes()).c_str());
			format.SetAttribute("cropxmin", ToString(QGetRenderContext()->pImage()->CropWindowXMin()).c_str());
			format.SetAttribute("cropxmax", ToString(QGetRenderContext()->pImage()->CropWindowXMax()).c_str());
			format.SetAttribute("cropymin", ToString(QGetRenderContext()->pImage()->CropWindowYMin()).c_str());
			format.SetAttribute("cropymax", ToString(QGetRenderContext()->pImage()->CropWindowYMax()).c_str());
			format.SetAttribute("cropymax", ToString(QGetRenderContext()->pImage()->CropWindowYMax()).c_str());
			format.SetAttribute("elementsize", ToString(datasize).c_str());
			root.InsertEndChild(format);
			doc.InsertEndChild(root);

			std::ostringstream strFormat;
			strFormat << doc;

			send(m_Socket, strFormat.str().c_str(), strFormat.str().length(), 0 );
		}
		else if((reqElement = reqHandle.FirstChild("aqsis:request").FirstChild("aqsis:bucket").Element()) != NULL)
		{
			// Get the index from the request.
			TqInt index;
			reqElement->Attribute("index", &index);

			// Check if the bucket is available.
			CqBucketDiskStore::SqBucketDiskStoreRecord* precord;
			while((precord = m_pManager->getDiskStore().RetrieveBucketIndex(index)) == NULL)
			{
				// Register an interest in the bucket with the manager.
				boost::mutex monitor;
				boost::mutex::scoped_lock lk(monitor);
				m_pManager->BucketReadyCondition(index)->wait(lk);
			}
			precord = m_pManager->getDiskStore().RetrieveBucketIndex(index);
			// Construct a response
			TiXmlDocument doc;
			TiXmlElement root("aqsis:response");
			TiXmlElement bucket("aqsis:bucket");
			bucket.SetAttribute("xmin", ToString(precord->m_OriginX).c_str());
			bucket.SetAttribute("ymin", ToString(precord->m_OriginY).c_str());
			bucket.SetAttribute("xmaxp1", ToString(precord->m_Width + precord->m_OriginX).c_str());
			bucket.SetAttribute("ymaxp1", ToString(precord->m_Height + precord->m_OriginY).c_str());
			TqInt dataLen = precord->m_BucketLength - sizeof(CqBucketDiskStore::SqBucketDiskStoreRecord) + sizeof(float);
			char* data = new char[(((dataLen+3)/3)*4)+1];
			dataLen = b64_encode(data, reinterpret_cast<const char*>(precord->m_Data), dataLen);
			data[dataLen] = '\0';
			TiXmlText dataText(data);
			delete[](data);
			bucket.InsertEndChild(dataText);
			root.InsertEndChild(bucket);
			doc.InsertEndChild(root);

			std::ostringstream strFormat;
			strFormat << doc;

			unsigned long msgLen = htonl(strFormat.str().length());
			send(m_Socket, reinterpret_cast<const char*>(&msgLen), sizeof(unsigned long), 0);
			send(m_Socket, strFormat.str().c_str(), strFormat.str().length(), 0 );
		}
	}
//	std::cerr << debug << "Sender thread ended: " << reinterpret_cast<long>(this) << std::endl;
}

static const char *b64_tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char b64_pad = '=';

/* base64 encode a group of between 1 and 3 input chars into a group of
 * 4 output chars */
static void encode_group(char output[], const char input[], int n)
{
	unsigned char ingrp[3];

	ingrp[0] = n > 0 ? input[0] : 0;
	ingrp[1] = n > 1 ? input[1] : 0;
	ingrp[2] = n > 2 ? input[2] : 0;

	/* upper 6 bits of ingrp[0] */
	output[0] = n > 0 ? b64_tbl[ingrp[0] >> 2] : b64_pad;
	/* lower 2 bits of ingrp[0] | upper 4 bits of ingrp[1] */
	output[1] = n > 0 ? b64_tbl[((ingrp[0] & 0x3) << 4) | (ingrp[1] >> 4)] : b64_pad;
	/* lower 4 bits of ingrp[1] | upper 2 bits of ingrp[2] */
	output[2] = n > 1 ? b64_tbl[((ingrp[1] & 0xf) << 2) | (ingrp[2] >> 6)] : b64_pad;
	/* lower 6 bits of ingrp[2] */
	output[3] = n > 2 ? b64_tbl[ingrp[2] & 0x3f] : b64_pad;
}

int b64_encode(char *dest, const char *src, int len)
{
	int outsz = 0;

	while (len > 0) 
	{
		encode_group(dest + outsz, src, len > 3 ? 3 : len);
		len -= 3;
		src += 3;
		outsz += 4;
	}
	return outsz;
}


END_NAMESPACE( Aqsis )
