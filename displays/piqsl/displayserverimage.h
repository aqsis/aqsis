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

#include	"aqsis.h"
#include	"ndspy.h"
#include	"image.h"
#include	"socket.h"

START_NAMESPACE( Aqsis )

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
	{
		m_quantize[0] = m_quantize[1] = m_quantize[2] = m_quantize[3] = 0.0f;
	}
    CqDisplayServerImage() : CqImage() 
	{
		m_quantize[0] = m_quantize[1] = m_quantize[2] = m_quantize[3] = 0.0f;
	}
    virtual ~CqDisplayServerImage()
	{}

	/** Get a reference to the socket that is used to communicate with the piqsl display device.
 	 * \return				A reference to the socket.
 	 */
    CqSocket& socket()
    {
        return ( m_socket );
    }
	/** Get a const reference to the socket that is used to communicate with the piqsl display device.
 	 * \return				A reference to the socket.
 	 */
    const CqSocket& socket() const
    {
        return ( m_socket );
    }

	/** Close the connection to the piqsl display server.
	 */
	void close();

	/** Get the defined zero value used during quantisation.
 	 * \return			The zero value that will be used during quantisation.
 	 */
	TqFloat quantizeZero() const
	{
		return(m_quantize[0]);
	}
	/** Set the zero value to be used during quantisation.
 	 * \param zero		The zero value to use during quantisation.
 	 */
	void setQuantizeZero(TqFloat zero)
	{
		m_quantize[0] = zero;
	}
	/** Get the defined one value used during quantisation.
 	 * \return			The one value that will be used during quantisation.
 	 */
	TqFloat quantizeOne() const
	{
		return(m_quantize[1]);
	}
	/** Set the one value to be used during quantisation.
 	 * \param one		The one value to use during quantisation.
 	 */
	void setQuantizeOne(TqFloat one)
	{
		m_quantize[1] = one;
	}
	/** Get the defined min value used during quantisation.
 	 * \return			The min value that will be used during quantisation.
 	 */
	TqFloat quantizeMin() const
	{
		return(m_quantize[2]);
	}
	/** Set the min value to be used during quantisation.
 	 * \param min		The min value to use during quantisation.
 	 */
	void setQuantizeMin(TqFloat min)
	{
		m_quantize[2] = min;
	}
	/** Get the defined max value used during quantisation.
 	 * \return			The max value that will be used during quantisation.
 	 */
	TqFloat quantizeMax() const
	{
		return(m_quantize[3]);
	}
	/** Set the max value to be used during quantisation.
 	 * \param max		The max value to use during quantisation.
 	 */
	void setQuantizeMax(TqFloat max)
	{
		m_quantize[3] = max;
	}
	/** Set all quantisation values in one go.
 	 * \param quant		A 4 element array containin the zero, one, min and max values for quantisation.
 	 */
	void setQuantize(const TqFloat (&quant)[4])
	{
		m_quantize[0] = quant[0];
		m_quantize[1] = quant[1];
		m_quantize[2] = quant[2];
		m_quantize[3] = quant[3];
	}

	/** Accept a bucket of data from the piqsl display server.
 	 * The data will have been delived to piqsl as an XML packet, this function expects the data to
 	 * have been parsed and converted to plain binary data in machine format.
 	 * \param xmin		The minimum x value in image coordinates of the bucket.
 	 * \param xmaxplus1	One past the maximum x value in image coordinates of the bucket.
 	 * \param ymin		The minimum y value in image coordinates of the bucket.
 	 * \param ymaxplus1	One past the maximum y value in image coordinates of the bucket.
 	 */
    void acceptData(TqUlong xmin, TqUlong xmaxplus1, TqUlong ymin, TqUlong ymaxplus1, TqInt elementSize, const unsigned char* data);
	
	/** Save the image as a TIFF file to the given folder.
 	 * Used during saving a book, this ensures that the image, which otherwise is completely transient, existing
 	 * only in memory, gets saved to disk so that it can be later reloaded into Piqsl with the book.
 	 * \param folder	The folder on disk to store the TIFF file to.
 	 */
	virtual void serialise(const std::string& folder);
	/** Create an XML element that represents this image in a library XML file.
 	 * \return			A pointer to a generated TinyXML element containing all the data to be
 	 * 					added to the XML file.
 	 */
	virtual TiXmlElement* serialiseToXML();

	/** A helper function to reorder the channels from Aqsis.
	 * A helper function to reorder the channels that Aqsis sends to ensure that they are in the expected format
 	 * for display by piqsl, and subsequent saving to TIFF.
 	 */
	void reorderChannels();

private:
    CqSocket	m_socket;			///< Socket of the client.
	TqFloat		m_quantize[4];		///< Stored quantisation values, used during conversion for display.
};

END_NAMESPACE( Aqsis )

#endif	// DDSERVER_H_INCLUDED
