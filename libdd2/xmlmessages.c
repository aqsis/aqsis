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
		\brief Helper functions for processing the displays XML based messages.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	<stdio.h>
#include	<stdlib.h>
#include	"xmlmessages.h"

#ifdef _WIN32

#include	<process.h>
#include	<winsock2.h>

#else // _WIN32

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

#endif // !_WIN32

//---------------------------------------------------------------------
/** Receive a message on the given socket.
 *  Gets the message length first, and the ensures that the whole message is read.
 */

char* receiveXMLMessage(int socket)
{
	unsigned long msgLen;
	unsigned long tot = 0, left;
	char* req = 0;
	int n;

	n = recv(socket, (char*)&msgLen, sizeof(unsigned long), 0);
	// If recv returns 0, then the socket has been closed.
	if(n == 0)
		return(0);

	msgLen = ntohl(msgLen);
	left = msgLen;
	req = malloc(msgLen+1);
	while(left > 0)
	{
		n = recv(socket, &req[tot], left, 0);
		tot += n;
		left -= n;
	}
	req[msgLen] = '\0';

	return(req);
}


//---------------------------------------------------------------------
/** Send a given XML message on the socket.
 *  Sends the message length first, and then ensures that the whole message is sent.
 *  The message should be '\0' terminated.
 */
void sendXMLMessage(int socket, const char* msg)
{
	unsigned long len = strlen(msg);
	unsigned long msgLen = htonl(len);
	send(socket, (const char*)&msgLen, sizeof(unsigned long), 0);
	send(socket, msg, len, 0 );
}
