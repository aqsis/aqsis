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

//? Is context.h included already?
#ifndef LISTENER_H_INCLUDED
#define LISTENER_H_INCLUDED 1

#include	<vector>
#include	<string>

#ifdef AQSIS_SYSTEM_WIN32

#include	<winsock2.h>

#else // AQSIS_SYSTEM_WIN32

typedef int SOCKET;

#endif // !AQSIS_SYSTEM_WIN32

#include	"aqsis.h"
#include	"bucketcache.h"

#ifdef	AQSIS_SYSTEM_WIN32
#pragma warning(disable : 4275 4251)
#endif

#include	<boost/thread.hpp>

#define		AQSIS_LISTENER_PORT	27747	///< Aqsis display driver port ( AQSIS2 on phone keypad )

START_NAMESPACE( Aqsis )

class CqDDManager;


class CqDisplayListener
{
public:
    CqDisplayListener();
    ~CqDisplayListener();

    TqBool	Prepare( TqInt port, CqDDManager* manager );
	void	operator()();
    void	Close();
	void	FrameEnd();

	SOCKET	Socket()	{return(m_Socket);}

private:
    SOCKET	m_Socket;			///< Socket ID of the server.
    int m_Port;				///< Port number used by this server.
	boost::thread* m_listenerThread;
	std::vector<boost::thread*>	m_senderThreads;
	CqDDManager* m_pManager;
}
;


class CqSender
{
	public:
		CqSender(int socket, CqDDManager *manager);
		~CqSender()	{}

		void operator()();

	private:
		int	m_Socket;
		CqDDManager* m_pManager;
}
;

END_NAMESPACE( Aqsis )

#endif	// LISTENER_H_INCLUDED
