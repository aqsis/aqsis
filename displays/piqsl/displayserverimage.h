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
 * Class encapsulating the display driver server thread.
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

    CqSocket& socket()
    {
        return ( m_socket );
    }
    const CqSocket& socket() const
    {
        return ( m_socket );
    }

	void close();

	TqFloat quantizeZero() const
	{
		return(m_quantize[0]);
	}
	void setQuantizeZero(TqFloat zero)
	{
		m_quantize[0] = zero;
	}
	TqFloat quantizeOne() const
	{
		return(m_quantize[1]);
	}
	void setQuantizeOne(TqFloat one)
	{
		m_quantize[1] = one;
	}
	TqFloat quantizeMin() const
	{
		return(m_quantize[2]);
	}
	void setQuantizeMin(TqFloat min)
	{
		m_quantize[2] = min;
	}
	TqFloat quantizeMax() const
	{
		return(m_quantize[3]);
	}
	void setQuantizeMax(TqFloat max)
	{
		m_quantize[3] = max;
	}
	void setQuantize(const TqFloat (&quant)[4])
	{
		m_quantize[0] = quant[0];
		m_quantize[1] = quant[1];
		m_quantize[2] = quant[2];
		m_quantize[3] = quant[3];
	}

    void acceptData(TqUlong xmin, TqUlong xmaxplus1, TqUlong ymin, TqUlong ymaxplus1, TqInt elementSize, const unsigned char* data);
	
	virtual void serialise(const std::string& folder);
	virtual TiXmlElement* serialiseToXML();

	void reorderChannels();

private:
    CqSocket	m_socket;			///< Socket of the client.
    std::stringstream m_readbuf;
	TqFloat		m_quantize[4];
};

END_NAMESPACE( Aqsis )

#endif	// DDSERVER_H_INCLUDED
