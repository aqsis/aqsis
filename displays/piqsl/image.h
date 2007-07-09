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
		\brief The base class from which all piqsl images derive.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is ddclient.h included already?
#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED 1

#include	<vector>
#include	<string>
#include	<map>

#include	<boost/shared_ptr.hpp>
#include	<boost/function.hpp>
#include	<boost/thread/mutex.hpp>

#include	"aqsis.h"
#include	"sstring.h"
#include 	"tinyxml.h"

START_NAMESPACE( Aqsis )

class CqFramebuffer;

//---------------------------------------------------------------------
/** \class CqImage
 * Abstract base class for piqsl images.
 */

class CqImage
{
public:
	CqImage( const CqString& name ) : m_name(name), m_data(0), m_realData(0), m_frameWidth(0), m_frameHeight(0), m_imageWidth(0), m_imageHeight(0), m_originX(0), m_originY(0), m_elementSize(0)
	{} 
	CqImage() : m_data(0), m_realData(0), m_frameWidth(0), m_frameHeight(0), m_imageWidth(0), m_imageHeight(0), m_originX(0), m_originY(0), m_elementSize(0)
	{}
    virtual ~CqImage();

    virtual CqString&	name()
    {
        return ( m_name );
    }
    virtual void	setName( const CqString& name )
    {
        m_name = name;
    }
    virtual CqString&	filename()
    {
        return ( m_fileName );
    }
    virtual void	setFilename( const CqString& name )
    {
        m_fileName = name;
    }
	virtual TqUlong	frameWidth()
	{
		return( m_frameWidth );
	}
	virtual TqUlong frameHeight()
	{
		return( m_frameHeight );
	}
	virtual void setFrameSize(TqUlong width, TqUlong height)
	{
		m_frameWidth = width;
		m_frameHeight = height;
	}
	virtual const std::string& channelName(TqInt index)
	{
		assert(index < numChannels());
		return(m_channels[index].first);
	}
	virtual void setChannelName(TqInt index, const std::string& name)
	{
		assert(index < numChannels());
		m_channels[index].first = name;
	}
	virtual TqInt channelType(TqInt index)
	{
		assert(index < numChannels());
		return(m_channels[index].second);
	}
	virtual void setChannelType(TqInt index, TqInt type)
	{
		assert(index < numChannels());
		m_channels[index].second = type;
	}
	virtual TqInt numChannels() const
	{
		return( m_channels.size() );
	}
	virtual void addChannel(const std::string& name, TqInt type)
	{
		m_channels.push_back(std::pair<std::string, TqInt>(name,type));
	}
	virtual TqInt elementSize() const
	{
		return(m_elementSize);
	}
	virtual unsigned char* data()
	{
		return( m_data );
	}
	virtual TqUlong originX() const
	{
		return( m_originX );
	}
	virtual TqUlong originY() const
	{
		return( m_originY );
	}
	virtual void setOrigin(TqUlong originX, TqUlong originY)
	{
		m_originX = originX;
		m_originY = originY;
	}
	virtual TqUlong imageWidth() const
	{
		return( m_imageWidth );
	}
	virtual TqUlong imageHeight() const
	{
		return( m_imageHeight );
	}
	virtual void setImageSize(TqUlong imageWidth, TqUlong imageHeight)
	{
		m_imageWidth = imageWidth;
		m_imageHeight = imageHeight;
	}

	virtual void PrepareImageBuffer();
	
	virtual void setUpdateCallback(boost::function<void(int,int,int,int)> f);

	virtual void serialise(const std::string& folder)
	{}
	virtual TiXmlElement* serialiseToXML();
	void saveToTiff(const std::string& filename);
	void loadFromTiff(const std::string& filename);

	boost::mutex& mutex()
	{
		return(m_mutex);
	}

protected:
    CqString		m_name;			///< Display name.
    CqString		m_fileName;		///< File name.
	unsigned char*	m_data;			///< Buffer to store the 8bit data for display. 
	unsigned char*	m_realData;		///< Buffer to store the natural format image data.
	TqUlong			m_frameWidth;
	TqUlong			m_frameHeight;
	TqUlong			m_imageWidth;
	TqUlong			m_imageHeight;
	TqUlong			m_originX;
	TqUlong			m_originY;
	TqInt			m_elementSize;
	std::vector<std::pair<std::string, TqInt> >			m_channels;

	boost::function<void(int,int,int,int)> m_updateCallback;
	boost::mutex	m_mutex;
};

END_NAMESPACE( Aqsis )

#endif	// IMAGE_H_INCLUDED
