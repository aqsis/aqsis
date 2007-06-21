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
		\brief Declares an image class getting it's data from the Dspy server.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is ddclient.h included already?
#ifndef DISPLAYSERVERIMAGE_H_INCLUDED
#define DISPLAYSERVERIMAGE_H_INCLUDED 1

#include	<vector>
#include	<string>

#ifdef AQSIS_SYSTEM_WIN32

#include	<winsock2.h>

#else // AQSIS_SYSTEM_WIN32

typedef int SOCKET;

#endif // !AQSIS_SYSTEM_WIN32

#include	"aqsis.h"
#include	"ndspy.h"
#include	"image.h"

START_NAMESPACE( Aqsis )

struct SqDDMessageBase;
struct SqDDMessageData;

//---------------------------------------------------------------------
/** \class CqDDClient
 * Class encapsulating the display driver server thread.
 */

class CqDisplayServerImage : public CqImage
{
public:
    CqDisplayServerImage( const CqString name) : CqImage(name)
	{}
    CqDisplayServerImage() : CqImage() 
	{}
    virtual ~CqDisplayServerImage()
	{}

    void	close();
    void	sendData( void* buffer, TqInt len );
    void	sendMsg( SqDDMessageBase* pMsg );
    void	receive( void* buffer, TqInt len );
    /** Get a reference to the socket ID.
     */
    void	setSocket( SOCKET s )
    {
        m_socket = s;
    }
    SOCKET& socket()
    {
        return ( m_socket );
    }
    /** Get a reference to the socket ID.
     */
    const SOCKET& socket() const
    {
        return ( m_socket );
    }

    /** Get a reference to the current read buffer
      */
    std::stringstream& readbuf()
    {
        return ( m_readbuf );
    }

    virtual CqString&	filename()
    {
        return ( m_serialisedName );
    }
    virtual void	setFilename( const CqString& name )
    {
        m_serialisedName = name;
    }

    void acceptData(TqUlong xmin, TqUlong xmaxplus1, TqUlong ymin, TqUlong ymaxplus1, TqInt elementSize, const unsigned char* data);
	
	virtual void serialise(const std::string& folder);
	void saveToTiff(const std::string& filename);
	virtual TiXmlElement* serialiseToXML();

private:
    SOCKET	m_socket;			///< Socket ID of the client.
    std::stringstream m_readbuf;
	CqString	m_serialisedName;
};

END_NAMESPACE( Aqsis )

#endif	// DDSERVER_H_INCLUDED
