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
		\brief Implements the base message handling functionality required by display drivers.
		\author Paul C. Gregory (pgregory@aqsis.com)
		\author Timothy M. Shead (tshead@k-3d.com)
*/

#include	<iostream>
#include <memory>
#include	<strstream>

#include	"aqsis.h"

#ifdef AQSIS_SYSTEM_WIN32

#include	<winsock2.h>

#else // AQSIS_SYSTEM_WIN32

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef int SOCKET;
typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr* PSOCKADDR;

static const int INVALID_SOCKET = -1;
static const int SOCKET_ERROR = -1;
static const int SD_BOTH = 2;

#endif // !AQSIS_SYSTEM_WIN32

#include	<stdio.h>
#include	"displaydriver.h"

using namespace Aqsis;

TqInt Query(SOCKET s,SqDDMessageBase* pMsg);
TqInt Open(SOCKET s,SqDDMessageBase* pMsg);
TqInt Data(SOCKET s,SqDDMessageBase* pMsg);
TqInt Close(SOCKET s,SqDDMessageBase* pMsg);
TqInt HandleMessage(SOCKET s,SqDDMessageBase* pMsg);

START_NAMESPACE(Aqsis)

static void CloseSocket(SOCKET& Socket);

//----------------------------------------------------------------------
/** Receive a specified length of data from the specified socket.
 */

TqInt DDReceiveSome(TqInt s,void* buffer, TqInt len)
{
	TqInt tot=0, need=len;
	while(need>0)
	{
		TqInt n;
		if((n=recv(s,reinterpret_cast<char*>(buffer)+tot,need,0))>0)
		{
			need-=n;
			tot+=n;
		}
		else
			return(n);
	}
	return(tot);
}


//----------------------------------------------------------------------
/** Receive a formatted message from the specified socket.
 */

TqInt DDReceiveMsg(TqInt s, SqDDMessageBase*& pMsg)
{
	SqDDMessageBase msghdr;
	TqInt ret;

	pMsg=0;
	if((ret=DDReceiveSome(s,&msghdr,sizeof(msghdr)))>0)
	{
		// Allocate space for the message.
		char* msgbuffer=new char[msghdr.m_MessageLength];
		// Copy the header.
		memcpy(msgbuffer,&msghdr,ret);
		pMsg=reinterpret_cast<SqDDMessageBase*>(msgbuffer);

		if(ret<msghdr.m_MessageLength)
		{
			if((ret=DDReceiveSome(s,msgbuffer+ret,msghdr.m_MessageLength-ret))<=0)
			{
				delete[](msgbuffer);
				pMsg=0;
				return(ret);
			}
		}
		return(msghdr.m_MessageLength);
	}
	return(ret);
}



//----------------------------------------------------------------------
/** Send a specified length of data to the specified socket.
 */

TqInt DDSendSome(TqInt s,void* buffer, TqInt len)
{
	TqInt tot=0, need=len;
	while(need>0)
	{
		TqInt n;
		if((n=send(s,reinterpret_cast<char*>(buffer)+tot,need,0))>0)
		{
			need-=n;
			tot+=n;
		}
		else
			return(n);
	}
	return(tot);
}


//----------------------------------------------------------------------
/** Send a preformatted message to the specified port.
 */

TqInt DDSendMsg(TqInt s, SqDDMessageBase* pMsg)
{
	return(DDSendSome(s,pMsg,pMsg->m_MessageLength));
}


//----------------------------------------------------------------------
/** Enter a loop processing messages from the server.
 */

static SOCKET g_Socket = INVALID_SOCKET;

TqInt DDInitialise(const TqChar* phostname, TqInt port)
{
#ifdef AQSIS_SYSTEM_WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0),&wsaData);
#endif // AQSIS_SYSTEM_WIN32

	// Open a socket.
	g_Socket = socket(AF_INET,SOCK_STREAM,0);
	if(g_Socket != INVALID_SOCKET)
	{
		// If no host name specified, use the local machine.
		char hostName[255];
		if(phostname!=NULL)
			strcpy(hostName,phostname);
		else
			gethostname(hostName,255);

		hostent* pHost;
		pHost=gethostbyname(hostName);

		SOCKADDR_IN saTemp;
		memset(&saTemp,0,sizeof(saTemp));
		saTemp.sin_family=AF_INET;
		if(port<0)
			saTemp.sin_port=htons(27747);
		else
			saTemp.sin_port=htons(port);

		memcpy(&saTemp.sin_addr,pHost->h_addr,pHost->h_length);
		TqInt conret;
		if((conret=connect(g_Socket, (PSOCKADDR)&saTemp,sizeof(saTemp)))!=SOCKET_ERROR)
			return(0);
		else
		{
			CloseSocket(g_Socket);
			return(-1);
		}
	}
	else
		return(-1);
}

//----------------------------------------------------------------------
/** Process a single message synchronously (blocks), returning false iff there are no more messages to process
 */

bool DDProcessMessage()
{
	SqDDMessageBase* message = 0;
	const TqInt length = DDReceiveMsg(g_Socket, message);
	if(0 == length)
		{
			// Connection closed gracefully by server ...
			CloseSocket(g_Socket);
			return false;
		}
	else if(length < 0)
		{
			std::cerr << "Error reading from socket" << std::endl;
			CloseSocket(g_Socket);
			return false;
		}

	// Make sure our message gets deallocated ...
	std::auto_ptr<SqDDMessageBase> messagestorage(message);
	
	switch(message->m_MessageID)
		{
			case MessageID_FormatQuery:
				{
					if(0 != Query(g_Socket, message))
						{
							CloseSocket(g_Socket);
							return false;
						}
				}
				break;

			case MessageID_Open:
				{
					if(0 != Open(g_Socket, message))
						{
							CloseSocket(g_Socket);
							return false;
						}
				}
				break;

			case MessageID_Data:
				{
					if(0 != Data(g_Socket, message))
						{
							CloseSocket(g_Socket);
							return false;
						}
				}
				break;

			case MessageID_Close:
				{
					if(0 != Close(g_Socket, message))
						{
							CloseSocket(g_Socket);
							return false;
						}
				}
				break;

			default:
				{
					if(0 != HandleMessage(g_Socket, message))
						{
							CloseSocket(g_Socket);
							return false;
						}
				}
				break;
		}
		
	return true;
}

//----------------------------------------------------------------------
/**	Process a single message asynchronously (returns after the given timeout if there are no messages to process), 
		returning false iff there are no more messages to process
 */

bool DDProcessMessageAsync(const TqUint TimeoutSeconds, const TqUint TimeoutMicroSeconds)
{
	// Check to see if we have anything waiting ...
	fd_set files;
	FD_ZERO(&files);
	FD_SET(g_Socket, &files);

	timeval timeout;
	timeout.tv_sec = TimeoutSeconds;
	timeout.tv_usec = TimeoutMicroSeconds;
	
	const int ready = select(g_Socket + 1, &files, 0, 0, &timeout);
	if(0 == ready)
		return true;
		
	// We've got data waiting, so process it normally ...
	return DDProcessMessage();
}

//----------------------------------------------------------------------
/** Enter a loop processing messages from the server.
 */

TqInt DDProcessMessages()
{
	while(1)
	{
		SqDDMessageBase* pMsg;
		TqInt len;
		TqInt ret;
		if((len=DDReceiveMsg(g_Socket,pMsg))>0)
		{
			switch(pMsg->m_MessageID)
			{
				case MessageID_FormatQuery:
				{
					if((ret=Query(g_Socket,pMsg))!=0)
					{
						CloseSocket(g_Socket);
						return(ret);
					}
				}
				break;

				case MessageID_Open:
				{
					if((ret=Open(g_Socket,pMsg))!=0)
					{
						CloseSocket(g_Socket);
						return(ret);
					}
				}
				break;

				case MessageID_Data:
				{
					if((ret=Data(g_Socket,pMsg))!=0)
					{
						CloseSocket(g_Socket);
						return(ret);
					}
				}
				break;

				case MessageID_Close:
				{
					if((ret=Close(g_Socket,pMsg))!=0)
					{
						CloseSocket(g_Socket);
						return(ret);
					}
				}
				break;

				default:
				{
					if((ret=HandleMessage(g_Socket,pMsg))!=0)
					{
						CloseSocket(g_Socket);
						return(ret);
					}
				}
			}
			delete[](pMsg);
		}
		else if(len==0)
		{
			// Connection closed gracefully by server.
			CloseSocket(g_Socket);
			return(0);
		}
		else
		{
			std::cerr << "Error reading from socket" << std::endl;
			CloseSocket(g_Socket);
			return(-1);
		}
	}
}

static void CloseSocket(SOCKET& Socket)
{
#ifdef AQSIS_SYSTEM_WIN32
	int x=1;
	LINGER ling;
	ling.l_onoff=1;
	ling.l_linger=10;
	setsockopt(Socket,SOL_SOCKET,SO_LINGER,reinterpret_cast<const char*>(&ling),sizeof(ling));
	shutdown(Socket,SD_BOTH);
	closesocket(Socket);
#else // AQSIS_SYSTEM_WIN32
	shutdown(Socket, SD_BOTH);
	close(Socket);
#endif // !AQSIS_SYSTEM_WIN32

	Socket = INVALID_SOCKET;
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
