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
		\brief Display driver server handler.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is context.h included already?
#ifndef DDSERVER_H_INCLUDED
#define DDSERVER_H_INCLUDED 1

#include	<vector>
#include	<winsock.h>

#include	"aqsis.h"

#define		AQSIS_DD_PORT	27747	///< Aqsis display driver port ( AQSIS on phone keypad )

START_NAMESPACE(Aqsis)

struct SqDDMessageBase;

//---------------------------------------------------------------------
/** \class CqDDServer 
 * Class encapsulating the display driver server thread.
 */

class CqDDClient
{
	public:
				CqDDClient(const TqChar* name, const TqChar* type, TqInt mode) :
							m_strName(name),
							m_strType(type),
							m_Mode(mode),
							m_Socket(INVALID_SOCKET)
							{}
				~CqDDClient(){}

				/** Close the socket this client is associated with.
				 */
		void	Close()		{closesocket(m_Socket);}
		void	SendData(void* buffer, TqInt len);
		void	SendMsg(SqDDMessageBase* pMsg);
		void	Receive(void* buffer, TqInt len);
				/** Get a reference to the socket ID.
				 */
		void	SetSocket(SOCKET s)
							{m_Socket=s;}
		SOCKET&	Socket()	{return(m_Socket);}
				/** Get a reference to the socket ID.
				 */
	const SOCKET& Socket() const	
							{return(m_Socket);}

		CqString&	strName()	{return(m_strName);}
		void		SetstrName(const TqChar* name)
								{m_strName=name;}
		CqString&	strType()	{return(m_strType);}
		void		SetstrType(const TqChar* type)
								{m_strType=type;}
		TqInt		Mode()		{return(m_Mode);}
		void		SetstrMode(TqInt mode)
								{m_Mode=mode;}

	private:
		SOCKET		m_Socket;			///< Socket ID of the client.
		CqString	m_strName;			///< Display name.
		CqString	m_strType;			///< Display type.
		TqInt		m_Mode;				///< Display mode.
};


//---------------------------------------------------------------------
/** \class CqDDServer 
 * Class encapsulating the display driver server thread.
 */

class CqDDServer
{
	public:
				CqDDServer() : m_Socket(INVALID_SOCKET)
							{}
				CqDDServer(TqInt port);
				~CqDDServer();

		TqBool	Prepare(TqInt port);
		TqBool	Open();
				/** Close the socket this server is associated with.
				 */
		void	Close()		{closesocket(m_Socket);}
		TqBool	Bind(TqInt port);
		TqBool	Listen();
		TqBool	Accept(CqDDClient& dd);
				/** Get a reference to the socket ID.
				 */
		SOCKET&	Socket()	{return(m_Socket);}
				/** Get a reference to the socket ID.
				 */
	const SOCKET& Socket() const	
							{return(m_Socket);}

	private:
		SOCKET					m_Socket;			///< Socket ID of the server.
		unsigned long			m_AcceptThreadID;	///< Thread ID of the accept listener.
};



END_NAMESPACE(Aqsis)

#endif	// DDSERVER_H_INCLUDED