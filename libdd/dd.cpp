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

#include	"aqsis.h"

#include	<winsock.h>
#include	<stdio.h>
#include	"displaydriver.h"

using namespace Aqsis;

TqInt Query(SOCKET s,SqDDMessageBase* pMsg);
TqInt Open(SOCKET s,SqDDMessageBase* pMsg);
TqInt Data(SOCKET s,SqDDMessageBase* pMsg);
TqInt Close(SOCKET s,SqDDMessageBase* pMsg);
TqInt HandleMessage(SOCKET s,SqDDMessageBase* pMsg);


START_NAMESPACE(Aqsis)

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
				return(-1);
			}
		}
	}
	return(msghdr.m_MessageLength);
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
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0),&wsaData);

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
		saTemp.sin_family=AF_INET;
		if(port<0)
			saTemp.sin_port=htons(27747);
		else
			saTemp.sin_port=htons(port);

		memcpy(&saTemp.sin_addr,pHost->h_addr,pHost->h_length);
		if(connect(s,(PSOCKADDR)&saTemp,sizeof(saTemp))!=SOCKET_ERROR)
			return(0);
		else
		{
			closesocket(s);
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
						closesocket(s);
						return(ret);
					}
				}
				break;

				case MessageID_Open:
				{
					if((ret=Open(s,pMsg))!=0)
					{
						closesocket(s);
						return(ret);
					}
				}
				break;

				case MessageID_Data:
				{
					if((ret=Data(s,pMsg))!=0)
					{
						closesocket(s);
						return(ret);
					}
				}
				break;

				case MessageID_Close:
				{
					if((ret=Close(s,pMsg))!=0)
					{
						closesocket(s);
						return(ret);
					}
				}
				break;

				default:
				{
					if((ret=HandleMessage(s,pMsg))!=0)
					{
						closesocket(s);
						return(ret);
					}
				}
			}
			delete[](pMsg);
		}
		else if(len==0)
		{
			// Connection closed gracefully by server.
			closesocket(s);
			return(0);
		}
		else
		{
			std::cerr << "Error reading from socket" << std::endl;
			closesocket(s);
			return(-1);
		}
	}
}


//---------------------------------------------------------------------

END_NAMESPACE(Aqsis)
