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
#include	"base64.h"
#include	"xmlmessages.h"

#include	<boost/ref.hpp>

START_NAMESPACE( Aqsis )


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
	//m_listenerThread->join();
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
	char* req;
	while(( req = receiveXMLMessage(m_Socket) ) != 0 )
	{
		// Parse the XML message with TinyXML
		TiXmlDocument request;
		request.Parse(req);

		free(req);

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

			// Send the response to the client
			sendXMLMessage(m_Socket, strFormat.str().c_str());
		}
		else if((reqElement = reqHandle.FirstChild("aqsis:request").FirstChild("aqsis:bucket").Element()) != NULL)
		{
			// Get the index from the request.
			TqInt index;
			reqElement->Attribute("index", &index);

			// Check if the bucket is available.
			CqBucketDiskStore::SqBucketDiskStoreRecord* precord;
			if((precord = m_pManager->getDiskStore().RetrieveBucketIndex(index)) == NULL)
			{
				// Lock the buckets so that Aqsis can't slip in and ready the bucket while we are
				// preparing to notify interest.
				boost::mutex::scoped_lock lk(m_pManager->getBucketsLock());
				// Check again, just to see if the bucket has been entered while locking.
				if((precord = m_pManager->getDiskStore().RetrieveBucketIndex(index)) != NULL)
					break;
				// Register an interest in the bucket with the manager. This will block until the bucket is ready.
				m_pManager->BucketReadyCondition(index)->wait(lk);
				if((precord = m_pManager->getDiskStore().RetrieveBucketIndex(index)) == NULL)
				{
					std::cerr << error << "Bucket manager has notified the availability of bucket " << index << " but I still can't get it" << std::endl;
					throw( XqException("Listener") );
				}
			}
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

			std::ostringstream strBucket;
			strBucket << doc;

			// Send the bucket to the client.
			sendXMLMessage(m_Socket, strBucket.str().c_str());
		}
	}
}

END_NAMESPACE( Aqsis )
