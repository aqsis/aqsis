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
		\brief The base class from which all eqshibit images derive.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is ddclient.h included already?
#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED 1

#include	<vector>
#include	<string>

#include	"aqsis.h"

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** \class CqImage
 * Abstract base class for eqshibit images.
 */

class CqImage
{
public:
	CqImage( const CqString& name ) : m_name(name), m_data(0), m_frameWidth(0), m_frameHeight(0), m_imageWidth(0), m_imageHeight(0), m_originX(0), m_originY(0), m_channels(0)
	{} 
	CqImage() : m_data(0), m_frameWidth(0), m_frameHeight(0), m_imageWidth(0), m_imageHeight(0), m_originX(0), m_originY(0), m_channels(0)
	{}
    virtual ~CqImage()
	{
		free(m_data);
	}

    CqString&	name()
    {
        return ( m_name );
    }
    void	setName( const CqString& name )
    {
        m_name = name;
    }
	TqUlong	frameWidth()
	{
		return( m_frameWidth );
	}
	TqUlong frameHeight()
	{
		return( m_frameHeight );
	}
	void setFrameSize(TqUlong width, TqUlong height)
	{
		m_frameWidth = width;
		m_frameHeight = height;
	}
	TqInt channels()
	{
		return( m_channels );
	}
	void setChannels(TqInt channels)
	{
		m_channels = channels;
	}
	unsigned char* data()
	{
		return( m_data );
	}
	TqUlong originX() const
	{
		return( m_originX );
	}
	TqUlong originY() const
	{
		return( m_originY );
	}
	void setOrigin(TqUlong originX, TqUlong originY)
	{
		m_originX = originX;
		m_originY = originY;
	}
	TqUlong imageWidth() const
	{
		return( m_imageWidth );
	}
	TqUlong imageHeight() const
	{
		return( m_imageHeight );
	}
	void setImageSize(TqUlong imageWidth, TqUlong imageHeight)
	{
		m_imageWidth = imageWidth;
		m_imageHeight = imageHeight;
	}

	void PrepareImageBuffer()
	{
		m_data = reinterpret_cast<unsigned char*>(malloc( m_imageWidth * m_imageHeight * m_channels * sizeof(TqUchar)));
	}

protected:
    CqString		m_name;			///< Display name.
	unsigned char*	m_data;
	TqUlong			m_frameWidth;
	TqUlong			m_frameHeight;
	TqUlong			m_imageWidth;
	TqUlong			m_imageHeight;
	TqUlong			m_originX;
	TqUlong			m_originY;
	TqInt			m_channels;
};

END_NAMESPACE( Aqsis )

#endif	// IMAGE_H_INCLUDED
