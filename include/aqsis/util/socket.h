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
		\brief Declares the CqSocket class for handling files with RenderMan searchpath option support.
		\author Paul C. Gregory (pgregory@aqsis.org)

		Implementation is platform specific, existing in the platform folders.
*/

#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED 1

#include <aqsis/aqsis.h>

#include <sstream>

#include <boost/utility.hpp>

#ifdef AQSIS_SYSTEM_WIN32
#	include <winsock2.h>
#endif


namespace Aqsis {

#ifdef AQSIS_SYSTEM_WIN32
typedef SOCKET TqSocketId;
#else
typedef int TqSocketId;
#endif

//----------------------------------------------------------------------
/** \class CqSocket
 *  \brief Wrapper class around sockets based communications.
 */
#ifdef AQSIS_SYSTEM_WIN32
class AQSIS_UTIL_SHARE boost::noncopyable_::noncopyable;
#endif
class AQSIS_UTIL_SHARE CqSocket : boost::noncopyable
{
	public:
		CqSocket();
		CqSocket( int port );
		~CqSocket();

		static bool initialiseSockets();
		bool	prepare( int port );
		bool    prepare( const std::string hostname, int port);
		bool	open();
		/** Close the socket this server is associated with.
		 */
		void	close();
		bool	bind( int port );
		bool	bind(const std::string hostname, int port);
		bool	listen();
		bool	accept(CqSocket& socket);
		bool	connect(const std::string hostname, int port);
		int		sendData(const std::string& data) const;
		int		recvData(std::stringstream& buffer) const;
		/** Get the current port.
		 */
		int port() const
		{
			return ( m_port );
		}
		operator int()
		{
			return(m_socket);
		}
		operator bool();

	private:
		TqSocketId m_socket;  ///< Socket ID of the server.
		int m_port;         ///< Port number used by this server.
};

//-----------------------------------------------------------------------

} // namespace Aqsis

#endif	// !FILE_H_INCLUDED
