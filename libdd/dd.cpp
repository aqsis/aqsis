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
*/

#include	<iostream>
#include	<strstream>

#include	"aqsis.h"

#ifdef AQSIS_SYSTEM_WIN32

#include	<winsock2.h>

#else // AQSIS_SYSTEM_WIN32

#include <stdio.h>
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

static void CloseSocket(SOCKET s);

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

SOCKET s;

TqInt DDInitialise(const TqChar* phostname, TqInt port)
{
#ifdef AQSIS_SYSTEM_WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0),&wsaData);
#endif // AQSIS_SYSTEM_WIN32

	// Open a socket.
	s=socket(AF_INET,SOCK_STREAM,0);
	if(s!=INVALID_SOCKET)
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
		if((conret=connect(s,(PSOCKADDR)&saTemp,sizeof(saTemp)))!=SOCKET_ERROR)
			return(0);
		else
		{
			CloseSocket(s);
			return(-1);
		}
	}
	else
		return(-1);
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
		if((len=DDReceiveMsg(s,pMsg))>0)
		{
			switch(pMsg->m_MessageID)
			{
				case MessageID_FormatQuery:
				{
					if((ret=Query(s,pMsg))!=0)
					{
						CloseSocket(s);
						return(ret);
					}
				}
				break;

				case MessageID_Open:
				{
					if((ret=Open(s,pMsg))!=0)
					{
						CloseSocket(s);
						return(ret);
					}
				}
				break;

				case MessageID_Data:
				{
					if((ret=Data(s,pMsg))!=0)
					{
						CloseSocket(s);
						return(ret);
					}
				}
				break;

				case MessageID_Close:
				{
					if((ret=Close(s,pMsg))!=0)
					{
						CloseSocket(s);
						return(ret);
					}
				}
				break;

				default:
				{
					if((ret=HandleMessage(s,pMsg))!=0)
					{
						CloseSocket(s);
						return(ret);
					}
				}
			}
			delete[](pMsg);
		}
		else if(len==0)
		{
			// Connection closed gracefully by server.
			CloseSocket(s);
			return(0);
		}
		else
		{
			std::cerr << "Error reading from socket" << std::endl;
			CloseSocket(s);
			return(-1);
		}
	}
}


static void CloseSocket(SOCKET s)
{
#ifdef AQSIS_SYSTEM_WIN32
	int x=1;
	LINGER ling;
	ling.l_onoff=1;
	ling.l_linger=10;
	setsockopt(s,SOL_SOCKET,SO_LINGER,reinterpret_cast<const char*>(&ling),sizeof(ling));
	shutdown(s,SD_BOTH);
	closesocket(s);
#else // AQSIS_SYSTEM_WIN32
	shutdown(s, SD_BOTH);
	close(s);
#endif // !AQSIS_SYSTEM_WIN32
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
