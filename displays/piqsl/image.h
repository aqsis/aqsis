// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
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
		\brief The base class from which all piqsl images derive.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED 1

#include "aqsis.h"

#include <vector>
#include <string>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

#include "imagebuffer.h"
#include "tinyxml.h"

START_NAMESPACE( Aqsis )

class CqFramebuffer;

//---------------------------------------------------------------------
/** \class CqImage
 * Abstract base class for piqsl images.
 */

class CqImage
{
public:
	inline CqImage(const std::string& name = "");
    virtual ~CqImage();

	/** Get the name of the image.
	 * \return			The name of the image.
	 */
    virtual const std::string& name() const;
	/** Set the display name of the image.
	 * \param name		The new display name of the image.
	 */
    virtual void setName( const std::string& name );
	/** Get the filename of the image.
	 * \return			The filename of the image.
	 */
    virtual const std::string&	filename() const;
	/** Set the filename of the image.
	 * \param name		The new filename of the image.
	 */
    virtual void setFilename( const std::string& name );
	/** Set the Description
	 */
    virtual void setDescription( const std::string& software );
	/** Get the Description
	 */
    virtual const std::string& description() const;

	/** Get the frame width of the image.
	 * The frame width if the cropped rendered region, within the image.
	 * \return			The frame width of the image.
	 */
	virtual TqUlong	frameWidth() const;
	/** Get the frame height of the image.
	 * The frame height if the cropped rendered region, within the image.
	 * \return			The frame height of the image.
	 */
	virtual TqUlong frameHeight() const;
	/** Combined setter for frame size.
	 * \param width			The new frame width.
	 * \param height		The new frame height.
	 */
	virtual void setFrameSize(TqUlong width, TqUlong height);
	/** \brief Get the channel information list for the "real" data.
	 *
	 * \return The channel info list of channel names and types.
	 */
	inline const CqChannelInfoList& channelsInfo() const;
	/** \brief Connect a channel of the underlying data to the red display channel
	 */
	inline void connectChannelR(const std::string& chanName);
	/** \brief Connect a channel of the underlying data to the green display channel
	 */
	inline void connectChannelG(const std::string& chanName);
	/** \brief Connect a channel of the underlying data to the blue display channel
	 */
	inline void connectChannelB(const std::string& chanName);
	/** Get the number of channels in this image.
	 * \return				The number of channels.
	 */
	virtual TqUint numChannels() const;
	/** Get a pointer to the display data.
	 * The display buffer is simple 8 bits per channel data as displayable by an RGB image.
	 * \return				A pointer to the start of the display buffer.
	 */
	virtual boost::shared_ptr<const CqImageBuffer> displayBuffer() const;
	/** Get the origin of the cropped frame within the total image.
	 * \return				The origin of the frame.
	 */
	virtual TqUlong originX() const;
	/** Get the origin of the cropped frame within the total image.
	 * \return				The origin of the frame.
	 */
	virtual TqUlong originY() const;
	/** The the origin of the frame within the image.
	 * \param originx		The x origin within the image of the rendered frame.
	 * \param originy		The y origin within the image of the rendered frame.
	 */
	virtual void setOrigin(TqUlong originX, TqUlong originY);
	/** Get the total width of the image.
	 * \return			The total width of the image.
	 */
	virtual TqUlong imageWidth() const;
	/** Get the total height of the image.
	 * \return			The total height of the image.
	 */
	virtual TqUlong imageHeight() const;
	/** Set the total image size.
	 * \param imageWidth	The total image width.
	 * \param imageHeight	The total image height.
	 */
	virtual void setImageSize(TqUlong imageWidth, TqUlong imageHeight);

	/** \brief Setup the display and full-precision buffers.
	 *
	 * Presuming the image size has been setup, allocate the two buffers used
	 * by the image.
	 *
	 * \param channelsInfo - the list of channel information for the image.
	 */
	virtual void prepareImageBuffers(const CqChannelInfoList& channelsInfo);
	
	/** Setup a callback to be called when the image changes.
	 * \param f			A function that will be called with the region that has changed.
	 */
	virtual void setUpdateCallback(boost::function<void(int,int,int,int)> f);

	/** Save the image to the given folder.
	 * \note Overridden by derivations that manage their image data differently.
	 */
	virtual void serialise(const std::string& folder)
	{}
	/** Create an XML element representing this image for serialising to library files.
	 * \return			A pointer to a new TinyXML element structure.
	 */
	virtual TiXmlElement* serialiseToXML();
	/** Save the image to a TIFF file.
	 * \param filename	The name of the file to save the image into.
	 */
	void saveToTiff(const std::string& filename) const;
	/** Load the image from a TIFF file on disk.
	 * \param filename	The name of the TIFF file to load the image data from.
	 */
	void loadFromTiff(const std::string& filename);

protected:
	/** Get a reference to the unique mutex for this image.
	 * Used when locking the image during multithreaded operation.
	 * \return			A reference to the unique mutex for this image.
	 */
	boost::mutex& mutex() const;

    std::string		m_name;			///< Display name.
    std::string		m_fileName;		///< File name.
    std::string		m_description;		///< Description or Software' renderer name.
	boost::shared_ptr<CqImageBuffer> m_displayData;		///< Buffer to store the 8bit data for display. 
	boost::shared_ptr<CqImageBuffer> m_realData;	///< Buffer to store the natural format image data.
	TqUlong			m_frameWidth;	///< The width of the frame within the whole image.
	TqUlong			m_frameHeight;	///< The height of the frame within the whole image.
	TqUlong			m_imageWidth;	///< The total image width.
	TqUlong			m_imageHeight;	///< The total image height.
	TqUlong			m_originX;		///< The origin of the frame within the whole image.
	TqUlong			m_originY;		///< The origin of the frame within the whole image.
	TqChannelNameMap m_displayMap; ///< map from display to underlying channel names

	boost::function<void(int,int,int,int)> m_updateCallback;	///< A callback, called when an image changes.
	mutable boost::mutex m_mutex;	///< The unique mutex for this image.
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Implementation of CqImage inlines and templates

inline CqImage::CqImage( const std::string& name)
    : m_name(name),
    m_fileName(),
    m_description(),
	m_displayData(),
	m_realData(),
	m_frameWidth(0),
	m_frameHeight(0),
	m_imageWidth(0),
	m_imageHeight(0),
	m_originX(0),
	m_originY(0),
	m_displayMap(),
	m_updateCallback(),
	m_mutex()
{
	m_displayMap["r"] = "r";
	m_displayMap["g"] = "g";
	m_displayMap["b"] = "b";
}


inline void	CqImage::setDescription( const std::string& description )
{
	m_description = description;
}

inline const std::string&	CqImage::description( ) const
{
	return (m_description);
}

inline const std::string& CqImage::name() const
{
	return ( m_name );
}

inline void	CqImage::setName( const std::string& name )
{
	m_name = name;
}

inline const std::string& CqImage::filename() const
{
	return ( m_fileName );
}

inline void	CqImage::setFilename( const std::string& name )
{
	m_fileName = name;
}

inline TqUlong CqImage::frameWidth() const
{
	return( m_frameWidth );
}

inline TqUlong CqImage::frameHeight() const
{
	return( m_frameHeight );
}

inline void CqImage::setFrameSize(TqUlong width, TqUlong height)
{
	m_frameWidth = width;
	m_frameHeight = height;
}

inline const CqChannelInfoList& CqImage::channelsInfo() const
{
	return m_realData->channelsInfo();
}

inline TqUint CqImage::numChannels() const
{
	if(m_realData)
		return(m_realData->channelsInfo().numChannels());
	else
		return 0;
}

inline boost::shared_ptr<const CqImageBuffer> CqImage::displayBuffer() const
{
	return m_displayData;
}

inline TqUlong CqImage::originX() const
{
	return( m_originX );
}

inline TqUlong CqImage::originY() const
{
	return( m_originY );
}

inline void CqImage::setOrigin(TqUlong originX, TqUlong originY)
{
	m_originX = originX;
	m_originY = originY;
}

inline TqUlong CqImage::imageWidth() const
{
	return( m_imageWidth );
}

inline TqUlong CqImage::imageHeight() const
{
	return( m_imageHeight );
}

inline void CqImage::setImageSize(TqUlong imageWidth, TqUlong imageHeight)
{
	m_imageWidth = imageWidth;
	m_imageHeight = imageHeight;
}

inline boost::mutex& CqImage::mutex() const
{
	return(m_mutex);
}


END_NAMESPACE( Aqsis )

#endif	// IMAGE_H_INCLUDED
