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
		\brief Implements an image class getting it's data from the Dspy server.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/


#include	"aqsis.h"


#include	<fstream>
#include	<map>
#include	<algorithm>

#include	<boost/archive/iterators/base64_from_binary.hpp>
#include	<boost/archive/iterators/transform_width.hpp>
#include	<boost/archive/iterators/insert_linebreaks.hpp>

#ifdef AQSIS_SYSTEM_WIN32
#include	<process.h>
#include	<signal.h>
#else // !AQSIS_SYSTEM_WIN32
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<netinet/in.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/wait.h>
#include	<errno.h>

static const int INVALID_SOCKET = -1;
static const int SD_BOTH = 2;
static const int SOCKET_ERROR = -1;

typedef sockaddr_in SOCKADDR_IN;
typedef sockaddr* PSOCKADDR;
#endif // AQSIS_SYSTEM_WIN32

#include	"file.h"
#include	"displayserverimage.h"
#include	"aqsismath.h"

START_NAMESPACE( Aqsis )

//---------------------------------------------------------------------
/** Close the socket this client is associated with.
 */
void CqDisplayServerImage::close()
{
	m_socket.close();
}


//----------------------------------------------------------------------

// Tricky macros from [Smith 1995] (see ref below)

/** Compute int((a/255.0)*b) with only integer arithmetic.  Assumes a, b are
 * between 0 and 255.
 */
#define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
/** Compute a composite of alpha-premultiplied values using integer arithmetic.
 *
 * Assumes p, q are between 0 and 255.
 *
 * \return int(q + (1-a/255.0)*p)
 */
#define INT_PRELERP(p, q, a, t) ( (p) + (q) - INT_MULT( a, p, t) )

/** \brief Composite two integers with a given alpha
 *
 * See for eg:
 * [Smith 1995]  Alvy Ray Smith, "Image Compositing Fundamentals", Technical
 * Memo 4, 1995.  ftp://ftp.alvyray.com/Acrobat/4_Comp.pdf
 *
 * \param r - top surface; alpha-premultiplied.  Assumed to be between 0 and 255.
 * \param R - bottom surface; alpha-premultiplied
 * \param alpha - alpha for top surface
 */
void CompositeAlpha(TqInt r, unsigned char &R, unsigned char alpha )
{ 
	TqInt t;
	TqInt R1 = static_cast<TqInt>(INT_PRELERP( R, r, alpha, t ));
	R = Aqsis::clamp( R1, 0, 255 );
}


void CqDisplayServerImage::acceptData(TqUlong xmin, TqUlong xmaxplus1, TqUlong ymin, TqUlong ymaxplus1, TqInt elementSize, const unsigned char* bucketData )
{
	assert(elementSize == m_elementSize);

	TqUlong xmin__ = Aqsis::max((xmin - originX()), 0UL);
	TqUlong ymin__ = Aqsis::max((ymin - originY()), 0UL);
	TqUlong xmaxplus1__ = Aqsis::min((xmaxplus1 - originX()), imageWidth());
	TqUlong ymaxplus1__ = Aqsis::min((ymaxplus1 - originY()), imageWidth());
	TqUlong bucketlinelen = elementSize * (xmaxplus1 - xmin); 
	TqUlong realLineLen = m_elementSize * imageWidth();
	
	boost::mutex::scoped_lock lock(mutex());

	const unsigned char* pdatarow = bucketData;
	// Calculate where in the bucket we are starting from if the window is cropped.
	TqInt row = 0;
	if(originY() > static_cast<TqUlong>(ymin))
		row = originY() - ymin;
	TqInt col = 0;
	if(originX() > static_cast<TqUlong>(xmin))
		col = originX() - xmin;
	pdatarow += (row * bucketlinelen) + (col * elementSize);

	if( data() && xmin__ >= 0 && ymin__ >= 0 && xmaxplus1__ <= imageWidth() && ymaxplus1__ <= imageHeight() )
	{
		TqUlong y;
		boost::shared_array<unsigned char> unrolled = m_data;
		boost::shared_array<unsigned char> realData = m_realData;

		for ( y = ymin__; y < ymaxplus1__; y++ )
		{
			TqUlong x;
			const unsigned char* _pdatarow = pdatarow;
			for ( x = xmin__; x < xmaxplus1__; x++ )
			{
				TqInt displayOffset = numChannels() * (( y * imageWidth() ) +  x );
				TqInt storageOffset = (( y * realLineLen ) + ( x * m_elementSize ) );
				TqUchar alpha = 255;
				/// \todo: Work out how to read alpha from the bucket data, taking into account sizes.
				CqImage::TqChannelListIterator channel;
				for(channel = m_channels.begin(); channel != m_channels.end(); ++channel)
				{
					switch(channel->second)
					{
						case PkDspyUnsigned16:
						{
							const TqUshort *svalue = reinterpret_cast<const TqUshort *>(_pdatarow);
							reinterpret_cast<TqUshort*>(&realData[storageOffset])[0] = svalue[0];
							CompositeAlpha((TqInt) (svalue[0]>>8), unrolled[displayOffset], alpha);
							_pdatarow += sizeof(TqUshort);
							storageOffset += sizeof(TqUshort);
						}
						break;
						case PkDspySigned16:
						{
							const TqShort *svalue = reinterpret_cast<const TqShort *>(_pdatarow);
							reinterpret_cast<TqShort*>(&realData[storageOffset])[0] = svalue[0];
							CompositeAlpha((TqInt) (svalue[0]>>7), unrolled[displayOffset], alpha);
							_pdatarow += sizeof(TqShort);
							storageOffset += sizeof(TqShort);
						}
						break;
						case PkDspyUnsigned32:
						{

							const TqUlong *lvalue = reinterpret_cast<const TqUlong *>(_pdatarow);
							reinterpret_cast<TqUlong*>(&realData[storageOffset])[0] = lvalue[0];
							CompositeAlpha((TqInt) (lvalue[0]>>24), unrolled[displayOffset], alpha);
							_pdatarow += sizeof(TqUlong);
							storageOffset += sizeof(TqUlong);
						}
						break;
						case PkDspySigned32:
						{

							const TqLong *lvalue = reinterpret_cast<const TqLong *>(_pdatarow);
							reinterpret_cast<TqLong*>(&realData[storageOffset])[0] = lvalue[0];
							CompositeAlpha((TqInt) (lvalue[0]>>23), unrolled[displayOffset], alpha);
							_pdatarow += sizeof(TqLong);
							storageOffset += sizeof(TqLong);
						}
						break;

						case PkDspyFloat32:
						{
							const TqFloat *fvalue = reinterpret_cast<const TqFloat *>(_pdatarow);
							reinterpret_cast<TqFloat*>(&realData[storageOffset])[0] = fvalue[0];
							CompositeAlpha((TqInt) (fvalue[0]*255.0), unrolled[displayOffset], alpha);
							_pdatarow += sizeof(TqFloat);
							storageOffset += sizeof(TqFloat);
						}
						break;

						case PkDspySigned8:
						case PkDspyUnsigned8:
						default:
						{
							const TqUchar *cvalue = reinterpret_cast<const TqUchar *>(_pdatarow);
							reinterpret_cast<TqUchar*>(&realData[storageOffset])[0] = cvalue[0];
							CompositeAlpha((TqInt) cvalue[0], unrolled[displayOffset], alpha);
							_pdatarow += sizeof(TqUchar);
							storageOffset += sizeof(TqUchar);
						}
						break;
					}
					++displayOffset;
				}
			}
			pdatarow += bucketlinelen;
		}
		if(m_updateCallback)
			m_updateCallback(xmin__, ymin__, xmaxplus1__-xmin__, ymaxplus1__-ymin__);
	}
}

void CqDisplayServerImage::serialise(const std::string& folder)
{
	// Generate a unique name for the managed image in the specified folder.
	std::string _ext = CqFile::extension(name());
	std::string _basename = CqFile::baseName(name());
	std::stringstream strFilename;
	strFilename << folder << CqFile::pathSep() << name();
	FILE* tfile;
	int index = 1;
	while((tfile = fopen(strFilename.str().c_str(), "r")) != NULL)
	{
		fclose(tfile);
		strFilename.clear();
		strFilename.str("");
		strFilename << folder << CqFile::pathSep() << _basename<< "." << index << _ext;
		index++;
	}
		
	setFilename(strFilename.str());
	saveToTiff(strFilename.str());
}



// Define a base64 encoding stream iterator using the boost archive data flow iterators.
typedef 
    boost::archive::iterators::insert_linebreaks<         // insert line breaks every 72 characters
        boost::archive::iterators::base64_from_binary<    // convert binary values ot base64 characters
            boost::archive::iterators::transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
                const char *,
                6,
                8
            >
        > 
        ,72
    > 
    base64_text; // compose all the above operations in to a new iterator

TiXmlElement* CqDisplayServerImage::serialiseToXML()
{
	TiXmlElement* imageXML = new TiXmlElement("Image");

	TiXmlElement* typeXML = new TiXmlElement("Type");
	TiXmlText* typeText = new TiXmlText("managed");
	typeXML->LinkEndChild(typeText);
	imageXML->LinkEndChild(typeXML);

	TiXmlElement* nameXML = new TiXmlElement("Name");
	TiXmlText* nameText = new TiXmlText(name());
	nameXML->LinkEndChild(nameText);
	imageXML->LinkEndChild(nameXML);

	if(filename().empty())
	{
		TiXmlElement* dataXML = new TiXmlElement("Bitmap");
		std::stringstream base64Data;
		size_t dataLen = m_imageWidth * m_imageHeight * numChannels() * sizeof(TqUchar);
		std::copy(	base64_text(BOOST_MAKE_PFTO_WRAPPER(m_data.get())), 
					base64_text(BOOST_MAKE_PFTO_WRAPPER(m_data.get() + dataLen)), 
					std::ostream_iterator<char>(base64Data));
		TiXmlText* dataTextXML = new TiXmlText(base64Data.str());
		dataTextXML->SetCDATA(true);
		dataXML->LinkEndChild(dataTextXML);
		imageXML->LinkEndChild(dataXML);
	}
	else
	{
		TiXmlElement* filenameXML = new TiXmlElement("Filename");
		TiXmlText* filenameText = new TiXmlText(filename());
		filenameXML->LinkEndChild(filenameText);
		imageXML->LinkEndChild(filenameXML);
	}

	return(imageXML);
}

void CqDisplayServerImage::reorderChannels()
{
	// If there are "r", "g", "b" and "a" channels, ensure they
	// are in the expected order.
	const char* elements[] = { "r", "g", "b", "a" };
	TqInt numElements = sizeof(elements) / sizeof(elements[0]);
	for(int elementIndex = 0; elementIndex < numElements; ++elementIndex)
	{
		for(CqImage::TqChannelListIterator channel = m_channels.begin(); channel != m_channels.end(); ++channel)
		{
			// If this entry in the channel list matches one in the expected list, 
			// move it to the right point in the channel list.
			if(channel->first.compare(elements[elementIndex]) == 0)
			{
				CqImage::TqChannel temp = m_channels[elementIndex];
				m_channels[elementIndex] = *channel;
				*channel = temp;
				break;
			}
		}
	}
}

END_NAMESPACE( Aqsis )
