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
#include	"boost/shared_ptr.hpp"

START_NAMESPACE( Aqsis )

struct SqDDMessageBase;
class CqDisplayServerImage;

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
    TqBool	Accept( boost::shared_ptr<CqDisplayServerImage> dd );
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
};



END_NAMESPACE( Aqsis )

#endif	// DDSERVER_H_INCLUDED
