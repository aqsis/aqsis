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
#include	<string>

#ifdef AQSIS_SYSTEM_WIN32

#include	<winsock2.h>

#else // AQSIS_SYSTEM_WIN32

typedef int SOCKET;

#endif // !AQSIS_SYSTEM_WIN32

#include	"aqsis.h"

#define		AQSIS_DD_PORT	27747	///< Aqsis display driver port ( AQSIS on phone keypad )

START_NAMESPACE( Aqsis )

struct SqDDMessageBase;

//---------------------------------------------------------------------
/** \class CqDDClient
 * Class encapsulating the display driver server thread.
 */

class CqDDClient
{
	public:
		CqDDClient( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt modeID, TqInt dataoffset = 0, TqInt datasize = 0 );
		~CqDDClient();

		/** Close the socket this client is associated with.
		 */
		void	Close();
		void	SendData( void* buffer, TqInt len );
		void	SendMsg( SqDDMessageBase* pMsg );
		void	Receive( void* buffer, TqInt len );
		/** Get a reference to the socket ID.
		 */
		void	SetSocket( SOCKET s )
		{
			m_Socket = s;
		}
		SOCKET& Socket()
		{
			return ( m_Socket );
		}
		/** Get a reference to the socket ID.
		 */
		const SOCKET& Socket() const
		{
			return ( m_Socket );
		}

		CqString&	strName()
		{
			return ( m_strName );
		}
		TqUlong&	hMode()
		{
			return ( m_hMode );
		}
		void	SetstrName( const TqChar* name )
		{
			m_strName = name;
		}
		CqString&	strType()
		{
			return ( m_strType );
		}
		void	SetstrType( const TqChar* type )
		{
			m_strType = type;
		}
		CqString&	strMode()
		{
			return ( m_strMode );
		}
		void	SetstrMode( const TqChar* mode )
		{
			m_strMode = mode;
		}
		TqInt GetdataOffset() const
		{
			return( m_dataOffset );
		}
		TqInt GetdataSize() const
		{
			return( m_dataSize );
		}
		TqInt GetmodeID() const
		{
			return( m_modeID );
		}

	private:
		SOCKET	m_Socket;			///< Socket ID of the client.
		CqString	m_strName;			///< Display name.
		CqString	m_strType;			///< Display type.
		CqString	m_strMode;			///< Display mode.
		TqInt		m_modeID;
		TqInt		m_dataOffset;
		TqInt		m_dataSize;
		TqUlong m_hMode;
}
;


//---------------------------------------------------------------------
/** \class CqDDServer
 * Class encapsulating the display driver server thread.
 */

class CqDDServer
{
	public:
		CqDDServer();
		CqDDServer( TqInt port );
		~CqDDServer();

		TqBool	Prepare( TqInt port );
		TqBool	Open();
		/** Close the socket this server is associated with.
		 */
		void	Close();
		TqBool	Bind( TqInt port );
		TqBool	Listen();
		TqBool	Accept( CqDDClient& dd );
		/** Get a reference to the socket ID.
		 */
		SOCKET&	Socket()
		{
			return ( m_Socket );
		}
		/** Get a reference to the socket ID.
		 */
		const SOCKET& Socket() const
		{
			return ( m_Socket );
		}
		/** Get the current port.
		 */
		int getPort() const
		{
			return ( m_Port );
		}
	private:
		SOCKET	m_Socket;			///< Socket ID of the server.
		int m_Port;				///< Port number used by this server.
}
;


//---------------------------------------------------------------------
/** \class CqDDManager
 * Class providing display device management to the renderer.
 */

class CqDDManager : public IqDDManager
{
	public:
		CqDDManager();
		virtual ~CqDDManager();

		// Overridden from IqDDManager
		virtual	TqInt	Initialise();
		virtual	TqInt	Shutdown();
		virtual	TqInt	AddDisplay( const TqChar* name, const TqChar* type, const TqChar* mode, TqInt compression, TqInt quality, TqInt modeID, TqInt dataOffset, TqInt dataSize );
		virtual	TqInt	ClearDisplays();
		virtual	TqInt	OpenDisplays();
		virtual	TqInt	CloseDisplays();
		virtual	TqInt	DisplayBucket( IqBucket* pBucket );
		virtual TqBool	fDisplayNeeds( const TqChar* var );
		virtual TqInt	Uses();

		void	LoadDisplayLibrary( CqDDClient& dd );
		void	InitialiseDisplayNameMap();

	private:
		std::string	GetStringField( const std::string& s, int idx );

	private:
		CqDDServer	m_DDServer;
		std::vector<CqDDClient>	m_aDisplayRequests;		///< Array of requested display drivers.
		std::vector<TqInt> m_aDisplayCompression;	///< Array of requested compression drivers.
		std::vector<TqInt> m_aDisplayQuality;	///< Array of requested quality drivers.
}
;


END_NAMESPACE( Aqsis )

#endif	// DDSERVER_H_INCLUDED
