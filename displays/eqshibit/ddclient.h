// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
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
		\brief Display driver client handler.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is ddclient.h included already?
#ifndef DDCLIENT_H_INCLUDED
#define DDCLIENT_H_INCLUDED 1

#include	<vector>
#include	<string>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>

#ifdef AQSIS_SYSTEM_WIN32

#include	<winsock2.h>

#else // AQSIS_SYSTEM_WIN32

typedef int SOCKET;

#endif // !AQSIS_SYSTEM_WIN32

#include	"aqsis.h"
#include	"ndspy.h"

class Fl_FrameBuffer_Widget;

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
	CqDDClient() : m_data(0) 
	{}
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
	TqUlong	GetWidth()
	{
		return(m_width);
	}
	TqUlong GetHeight()
	{
		return( m_height );
	}
	void SetWidth(TqUlong width)
	{
		m_width = width;
	}
	void SetHeight(TqUlong height)
	{
		m_height = height;
	}
	TqInt GetChannels()
	{
		return( m_channels );
	}
	void SetChannels(TqInt channels)
	{
		m_channels = channels;
	}
	void* data()
	{
		return( m_data );
	}

	void PrepareImageBuffer()
	{
		m_data = reinterpret_cast<char*>(malloc( m_width * m_height * m_channels * sizeof(TqUchar)));
	}

private:
    SOCKET	m_Socket;			///< Socket ID of the client.
    CqString	m_strName;			///< Display name.
    CqString	m_strType;			///< Display type.
    CqString	m_strMode;			///< Display mode.
    TqInt		m_modeID;
    TqInt		m_dataOffset;
    TqInt		m_dataSize;
	TqUlong		m_width;
	TqUlong		m_height;
	TqInt		m_channels;
    TqUlong m_hMode;
	char*		m_data;
public:
	Fl_Window*	m_theWindow;
	Fl_FrameBuffer_Widget* m_uiImageWidget;
	Fl_RGB_Image*	m_uiImage;
};

END_NAMESPACE( Aqsis )

#endif	// DDSERVER_H_INCLUDED
