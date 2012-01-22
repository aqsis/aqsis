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
		\brief Implements the system specific parts of the CqSocket class for wrapping socket communications.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#include	<aqsis/util/socket.h>

#include	<signal.h>

#include	<aqsis/util/logging.h>

namespace Aqsis {

//---------------------------------------------------------------------
/** Default constructor.
 */

CqSocket::CqSocket() :
        m_socket( INVALID_SOCKET ), 
		m_port( 0 )
{}

//---------------------------------------------------------------------
/** Constructor, takes a port no. and prepares the socket to accept clients.
 */
CqSocket::CqSocket( int port )
{
    prepare( port );
}


//---------------------------------------------------------------------
/** Destructor, close all connected client sockets.
 */
CqSocket::~CqSocket()
{
    close();
}

bool CqSocket::initialiseSockets()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		Aqsis::log() << error << "Error initializing sockets, code: " << err << std::endl;
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
	return(true);
}

//---------------------------------------------------------------------
/** Close the servers socket, waiting for any connectsion acticity to finish first,
 */
void CqSocket::close()
{
    int x = 1;
    //setsockopt( m_socket, SOL_SOCKET, SO_DONTLINGER, reinterpret_cast<const char*>( &x ), sizeof( x ) );
    shutdown( m_socket, SD_BOTH );
    closesocket( m_socket );

    m_socket = INVALID_SOCKET;
}


//---------------------------------------------------------------------
/** Prepare the socket to accept client connections.
 * \param port Integer port number to use.
 */

bool	CqSocket::prepare( int port )
{
    return prepare(std::string("0.0.0.0"), port);
}

bool	CqSocket::prepare( const std::string addr, int port )
{
    if ( open() )
        if ( bind( addr, port ) )
            if ( listen() )
                return ( true );
    return ( false );
}


//---------------------------------------------------------------------
/** Create the socket.
 */

bool CqSocket::open()
{
	m_socket = socket( AF_INET, SOCK_STREAM, 0 );

    if ( m_socket == INVALID_SOCKET )
    {
        TqInt err = WSAGetLastError();
        Aqsis::log() << error << "Error opening DD server socket " << err << std::endl;
        return ( false );
    }

    TqInt x = 1;
    setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>( &x ), sizeof( x ) );
    SetHandleInformation( ( HANDLE ) m_socket, HANDLE_FLAG_INHERIT, 0 );

    return ( true );
}


//---------------------------------------------------------------------
/** Bind the socket to a specified port.
 */

bool CqSocket::bind( TqInt port  )
{
    return bind(std::string("0.0.0.0"), port);
}

bool CqSocket::bind( const std::string hostname, TqInt port )
{
    SOCKADDR_IN saTemp;
    memset( &saTemp, 0, sizeof( saTemp ) );
    saTemp.sin_family = AF_INET;
    saTemp.sin_port = htons( port );
    saTemp.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ( ::bind( m_socket, ( PSOCKADDR ) & saTemp, sizeof( saTemp ) ) == SOCKET_ERROR )
    {
		Aqsis::log() << error << "Error binding to socket" << std::endl;
		close();
		return ( false );
    }
    m_port = port;
    return ( true );
}


//---------------------------------------------------------------------
/** Prepare the socket to listen for client connections.
 */

bool CqSocket::listen()
{
    if ( ::listen( m_socket, 5 ) == SOCKET_ERROR )
    {
        Aqsis::log() << error << "Error listening to socket" << std::endl;
        close();
        return ( false );
    }
    return ( true );
}


//---------------------------------------------------------------------
/** Set ip the thread to wait for client connection requests.
 */

bool CqSocket::accept(CqSocket& socket)
{
	socket.close();
    SOCKET c;

    if ( ( c = ::accept( m_socket, NULL, NULL ) ) != INVALID_SOCKET )
	{
		socket.m_socket = c;
		return ( true );
	}
	else
		return ( false );
}

bool CqSocket::connect(const std::string hostname, int port)
{
	// Assert that this socket hasn't already been configured as a server.
	assert(m_socket == -1 && m_port == 0);

	m_socket = socket(AF_INET,SOCK_STREAM,0);
	sockaddr_in adServer;

	ZeroMemory((char *) &adServer, sizeof(adServer)); 
	adServer.sin_family = AF_INET;
	adServer.sin_addr.s_addr = inet_addr(hostname.c_str());
	if(adServer.sin_addr.s_addr == INVALID_SOCKET)
	{
		Aqsis::log() << error << "Invalid IP address" << std::endl;;
		return(false);
	};
	adServer.sin_port = htons(port);

	if( ::connect(m_socket,(const struct sockaddr*) &adServer, sizeof(sockaddr_in)))
	{
		close();
		m_socket = INVALID_SOCKET;
		return(false);
	}
	m_port = port;
	return(true); 
}


CqSocket::operator bool()
{
	return(m_socket != INVALID_SOCKET);
}


int	CqSocket::sendData(const std::string& data) const 
{
	TqInt tot = 0, need = data.length();
	while ( need > 0 )
	{
		TqInt n = send( m_socket, data.c_str() + tot, need, 0 );
		need -= n;
		tot += n;
	}
	// Send terminator too.
	send(m_socket, "\0", 1, 0);
	tot += 1;

	return( tot );
}


int	CqSocket::recvData(std::stringstream& buffer) const
{
	char c;
	int count, total = 0;

	// Read a message
	while(1)	
	{
		// Read some more into the buffer
		count = recv(m_socket,&c,sizeof(char),0);
		if(count == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
	        Aqsis::log() << error << "Error reading from socket " << err << std::endl;
			break;
		}

		// Socket was closed gracefully.
		if(count == 0)
			break;

		if(c == '\0')
		{
			// Readbuf should now contain a complete message
			return(total);
		} 
		else 
		{
			buffer.put(c);
			total += count;
		}	
	}
	return count;
}

} // namespace Aqsis
//---------------------------------------------------------------------
