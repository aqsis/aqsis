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
		\brief Implements an image class getting it's data from the Dspy server.
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

static const int INVALID_SOCKET = -1;
static const int SD_BOTH = 2;
static const int SOCKET_ERROR = -1;

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr* PSOCKADDR;

#endif // !AQSIS_SYSTEM_WIN32

#include	"renderer.h"
#include	"render.h"
#include	"displaydriver.h"
#include	"displayserverimage.h"


START_NAMESPACE( Aqsis )


//---------------------------------------------------------------------
/** Close the socket this client is associated with.
 */
void CqDisplayServerImage::close()
{
#ifdef AQSIS_SYSTEM_WIN32
    int x = 1;
    setsockopt( m_socket, SOL_SOCKET, SO_DONTLINGER, reinterpret_cast<const char*>( &x ), sizeof( x ) );
    shutdown( m_socket, SD_BOTH );
    closesocket( m_socket );
#else // AQSIS_SYSTEM_WIN32
    shutdown( m_socket, SD_BOTH );
    ::close( m_socket );
#endif // !AQSIS_SYSTEM_WIN32

    m_socket = INVALID_SOCKET;
}



//---------------------------------------------------------------------
/** Send some data to the socket.
 * \param buffer Void pointer to the data to send.
 * \param len Integer length of the data in buffer.
 */

void CqDisplayServerImage::sendData( void* buffer, TqInt len )
{
    if ( m_socket == INVALID_SOCKET )
        return ;

    TqInt tot = 0, need = len;
    while ( need > 0 )
    {
        TqInt n = send( m_socket, reinterpret_cast<char*>( buffer ) + tot, need, 0 );
        need -= n;
        tot += n;
    }
}


//---------------------------------------------------------------------
/** Send a preconstructed message structure to this client.
 * \param pMsg Pointer to a SqDDMessageBase derive structure.
 */

void CqDisplayServerImage::sendMsg( SqDDMessageBase* pMsg )
{
    sendData( pMsg, pMsg->m_MessageLength );
}


//---------------------------------------------------------------------
/** Receive some data from the socket.
 * \param buffer Void pointer to the storage area for the data.
 * \param len Integer length of the data required.
 */

void CqDisplayServerImage::receive( void* buffer, TqInt len )
{
    TqInt tot = 0, need = len;
    while ( need > 0 )
    {
        TqInt n = recv( m_socket, reinterpret_cast<char*>( buffer ) + tot, need, 0 );
        need -= n;
        tot += n;
    }
}

END_NAMESPACE( Aqsis )
