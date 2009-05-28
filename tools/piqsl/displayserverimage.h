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
		\brief Declares an image class getting it's data from the Dspy server.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/

//? Is ddclient.h included already?
#ifndef DISPLAYSERVERIMAGE_H_INCLUDED
#define DISPLAYSERVERIMAGE_H_INCLUDED 1

#include	<aqsis/aqsis.h>

#include	<vector>
#include	<string>

#include	<aqsis/ri/ndspy.h>
#include	<aqsis/util/socket.h>

#include	"image.h"

namespace Aqsis {

struct SqDDMessageBase;
struct SqDDMessageData;

//---------------------------------------------------------------------
/** \class CqDDClient
 * Class implementing a specific image type that gets it's data from the 
 * piqsl display and aqsis.
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

	/** Get a reference to the socket that is used to communicate with the piqsl display device.
 	 * \return				A reference to the socket.
 	 */
	CqSocket& socket();
	/** Get a const reference to the socket that is used to communicate with the piqsl display device.
 	 * \return				A reference to the socket.
 	 */
    const CqSocket& socket() const;

	/** Close the connection to the piqsl display server.
	 */
	void close();

	/** \brief Accept a bucket of data from the piqsl display server.
 	 * The data will have been delived to piqsl as an XML packet, this function expects the data to
 	 * have been parsed and converted to plain binary data in machine format.
 	 * \param xmin		The minimum x value in image coordinates of the bucket.
 	 * \param xmaxplus1	One past the maximum x value in image coordinates of the bucket.
 	 * \param ymin		The minimum y value in image coordinates of the bucket.
 	 * \param ymaxplus1	One past the maximum y value in image coordinates of the bucket.
 	 */
    void acceptData(TqUlong xmin, TqUlong xmaxplus1, TqUlong ymin, TqUlong ymaxplus1, TqInt elementSize, const unsigned char* data);
	
	/** \brief Save the image as a TIFF file to the given folder.
 	 * Used during saving a book, this ensures that the image, which otherwise is completely transient, existing
 	 * only in memory, gets saved to disk so that it can be later reloaded into Piqsl with the book.
 	 * \param folder	The folder on disk to store the TIFF file to.
 	 */
	virtual void serialise(const boost::filesystem::path& folder);
	/** Create an XML element that represents this image in a library XML file.
 	 * \return			A pointer to a generated TinyXML element containing all the data to be
 	 * 					added to the XML file.
 	 */
	virtual TiXmlElement* serialiseToXML();

	/** \brief A helper function to reorder the channels from Aqsis.
	 * A helper function to reorder the channels that Aqsis sends to ensure that they are in the expected format
 	 * for display by piqsl, and subsequent saving to TIFF.
 	 */
	void reorderChannels();

private:
    CqSocket	m_socket;			///< Socket of the client.
};

// Implementations of inline functions.
inline CqSocket& CqDisplayServerImage::socket()
{
	return ( m_socket );
}

inline const CqSocket& CqDisplayServerImage::socket() const
{
	return ( m_socket );
}


} // namespace Aqsis

#endif	// DDSERVER_H_INCLUDED
