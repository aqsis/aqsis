// Aqsis
// Copyright (C) 2001, Paul C. Gregory and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)


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
    Q_OBJECT

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
	// virtual TiXmlElement* serialiseToXML();

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
