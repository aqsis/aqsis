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
		\brief Sockets based display device manager.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"

#include	<fstream>
#include	<map>
#include	"signal.h"

#ifdef AQSIS_SYSTEM_WIN32

#include	<process.h>

#else // AQSIS_SYSTEM_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <arpa/inet.h>

static const int INVALID_SOCKET = -1;
static const int SD_BOTH = 2;
static const int SOCKET_ERROR = -1;

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr* PSOCKADDR;

#endif // !AQSIS_SYSTEM_WIN32

#include	"renderer.h"
#include	"render.h"
#include	"ddserver.h"
#include	"displayserverimage.h"
#include	"imagebuffer.h"
#include	"rifile.h"


START_NAMESPACE( Aqsis )

#ifdef AQSIS_SYSTEM_POSIX

/// Handle cleanup of child processes
void sig_chld( int signo )
{
    pid_t pid;
    int stat;

    while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 );

    return ;
}

#endif // AQSIS_SYSTEM_POSIX


//---------------------------------------------------------------------
/** Default constructor.
 */

CqDDServer::CqDDServer() :
        m_Socket( INVALID_SOCKET )
{}

//---------------------------------------------------------------------
/** Constructor, takes a port no. and prepares the socket to accept clients.
 */

CqDDServer::CqDDServer( TqInt port )
{
    Prepare( port );
}


//---------------------------------------------------------------------
/** Destructor, close all connected client sockets.
 */

CqDDServer::~CqDDServer()
{
    Close();
}

//---------------------------------------------------------------------
/** Close the servers socket, waiting for any connectsion acticity to finish first,
 */

void CqDDServer::Close()
{
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

bool	CqDDServer::Prepare( TqInt port )
{
    if ( Open() )
        if ( Bind( port ) )
            if ( Listen() )
                return ( true );
    return ( false );
}


//---------------------------------------------------------------------
/** Create the socket.
 */

bool CqDDServer::Open()
{
#ifdef AQSIS_SYSTEM_WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return(false);
	}
	 
	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
	 
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		 HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		WSACleanup( );
		return(false); 
	}

	/* The WinSock DLL is acceptable. Proceed. */
#endif // AQSIS_SYSTEM_WIN32

	m_Socket = socket( AF_INET, SOCK_STREAM, 0 );

    if ( m_Socket == INVALID_SOCKET )
    {
#ifdef AQSIS_SYSTEM_WIN32
        TqInt err = WSAGetLastError();
        Aqsis::log() << error << "Error opening DD server socket " << err << std::endl;
#else
        Aqsis::log() << error << "Error opening DD server socket" << std::endl;
#endif // AQSIS_SYSTEM_WIN32
        return ( false );
    }

    TqInt x = 1;
    setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>( &x ), sizeof( x ) );

#ifdef AQSIS_SYSTEM_WIN32
    BOOL Ret = SetHandleInformation( ( HANDLE ) m_Socket, HANDLE_FLAG_INHERIT, 0 );
#endif // AQSIS_SYSTEM_WIN32

    return ( true );
}


//---------------------------------------------------------------------
/** Bind the socket to a specified port.
 */

bool CqDDServer::Bind( TqInt port )
{
    SOCKADDR_IN saTemp;
    memset( &saTemp, 0, sizeof( saTemp ) );
    saTemp.sin_family = AF_INET;
    saTemp.sin_port = htons( port );
    saTemp.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ( bind( m_Socket, ( PSOCKADDR ) & saTemp, sizeof( saTemp ) ) == SOCKET_ERROR )
    {
		Aqsis::log() << error << "Error binding to DD socket" << std::endl;
		Close();
		return ( false );
    }
    m_Port = port;
    return ( true );
}


//---------------------------------------------------------------------
/** Prepare the socket to listen for client connections.
 */

bool CqDDServer::Listen()
{
    if ( listen( m_Socket, 5 ) == SOCKET_ERROR )
    {
        Aqsis::log() << error << "Error listening to DD socket" << std::endl;
        Close();
        return ( false );
    }
    return ( true );
}


//---------------------------------------------------------------------
/** Set ip the thread to wait for client connection requests.
 */

bool CqDDServer::Accept( boost::shared_ptr<CqDisplayServerImage> dd )
{
    SOCKET c;

    if ( ( c = accept( Socket(), NULL, NULL ) ) != INVALID_SOCKET )
    {
        dd->setSocket( c );
        return( true );
    }
    return ( false );
}

END_NAMESPACE( Aqsis )
