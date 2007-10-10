// Aqsis
// Copyright (C) 1997 - 2007, Paul C. Gregory
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
 *
 * \brief A C++ wrapper around tiff files for the functions of interest in aqsis.
 *
 * \author Chris Foster
 */

#include "tiffdirhandle.h"

#include <tiffio.hxx>

#include "aqsismath.h"
#include "matrix.h"
#include "logging.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// XqUnknownTiffFormat
//------------------------------------------------------------------------------
/** \brief An exception 
 */
class COMMON_SHARE XqUnknownTiffFormat : public XqEnvironment
{
	public:
		XqUnknownTiffFormat (const std::string& reason, const std::string& detail,
			const std::string& file, const unsigned int line)
			: XqEnvironment(reason, detail, file, line)
		{ }

		XqUnknownTiffFormat (const std::string& reason,	const std::string& file,
			const unsigned int line)
			: XqEnvironment(reason, file, line)
		{ }

		virtual const char* description () const
		{
			return "XqUnknownTiffFormat";
		}

		virtual ~XqUnknownTiffFormat () throw ()
		{ }
};


//------------------------------------------------------------------------------
// CqTiffDirHandle
//------------------------------------------------------------------------------

CqTiffDirHandle::CqTiffDirHandle(boost::shared_ptr<CqTiffFileHandle> fileHandle, const tdir_t dirIdx)
	: m_fileHandle(fileHandle)
{
	fileHandle->setDirectory(dirIdx);
}

tdir_t CqTiffDirHandle::dirIndex() const
{
	return m_fileHandle->m_currDir;
}

void CqTiffDirHandle::fillHeader(CqTexFileHeader& header) const
{
	fillHeaderRequiredAttrs(header);
	fillHeaderOptionalAttrs(header);
	fillHeaderPixelLayout(header);
}

const char* getCompressionName(uint16 compressionType)
{
	switch(compressionType)
	{
		case COMPRESSION_NONE:
			return "none";
		case COMPRESSION_LZW:
			return "lzw";
		case COMPRESSION_JPEG:
			return "jpeg";
		case COMPRESSION_PACKBITS:
			return "packbits";
		case COMPRESSION_PIXARLOG:
			return "pixar_log";
		case COMPRESSION_DEFLATE:
			return "deflate";
		default:
			return "unknown";
	}
}

void CqTiffDirHandle::fillHeaderRequiredAttrs(CqTexFileHeader& header) const
{
	// Fill header with general metadata which won't affect the details of the
	// pixel memory layout.
	header.setAttribute<TqInt>("width", tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH));
	header.setAttribute<TqInt>("height", tiffTagValue<uint32>(TIFFTAG_IMAGELENGTH));
	header.setAttribute<bool>("isTiled", TIFFIsTiled(tiffPtr()));
	if(header.findAttribute<bool>("isTiled"))
	{
		header.setAttribute<TqInt>("tileWidth",
				tiffTagValue<uint32>(TIFFTAG_TILEWIDTH));
		header.setAttribute<TqInt>("tileHeight",
				tiffTagValue<uint32>(TIFFTAG_TILELENGTH));
	}
	// Get the compression type.
	header.setAttribute<std::string>("compression",
			getCompressionName(tiffTagValue<uint16>(TIFFTAG_COMPRESSION)) );
	// Compute pixel aspect ratio
	TqFloat xRes = 0;
	TqFloat yRes = 0;
	if(TIFFGetField(tiffPtr(), TIFFTAG_XRESOLUTION, &xRes)
			&& TIFFGetField(tiffPtr(), TIFFTAG_YRESOLUTION, &yRes))
	{
		// yRes/xRes should be the correct quantity corresponding to the
		// pixelAspectRatio used in OpenEXR.
		header.setAttribute<TqFloat>("pixelAspectRatio", yRes/xRes);
	}
	else
	{
		header.setAttribute<TqFloat>("pixelAspectRatio", 1.0f);
	}
	/// \todo Compute and save the data window
	/** Something like the "data window" in OpenEXR terminology can be
	 * obtained using
	 * TIFFTAG_XPOSITION TIFFTAG_PIXAR_IMAGEFULLWIDTH
	 * TIFFTAG_YPOSITION TIFFTAG_PIXAR_IMAGEFULLLENGTH
	 *
	 * This should allow sensible saving/loading of cropwindows...
	 */
}

// Extract an attribute from dirHandle and add it to header, if present.
template<typename Tattr, typename Ttiff>
void addAttributeToHeader(const char* attributName, ttag_t tag,
		CqTexFileHeader& header, const CqTiffDirHandle& dirHandle)
{
	Ttiff temp;
	if(TIFFGetField(dirHandle.tiffPtr(), tag, &temp))
		header.setAttribute<Tattr>(attributName, Tattr(temp));
}

void CqTiffDirHandle::fillHeaderOptionalAttrs(CqTexFileHeader& header) const
{
	// Add various descriptive strings to the header if they exist
	addAttributeToHeader<std::string,char*>("artist",
			TIFFTAG_ARTIST, header, *this);
	addAttributeToHeader<std::string,char*>("software",
			TIFFTAG_SOFTWARE, header, *this);
	addAttributeToHeader<std::string,char*>("hostname",
			TIFFTAG_HOSTCOMPUTER, header, *this);
	addAttributeToHeader<std::string,char*>("description",
			TIFFTAG_IMAGEDESCRIPTION, header, *this);
	addAttributeToHeader<std::string,char*>("dateTime",
			TIFFTAG_DATETIME, header, *this);

	// Add some matrix attributes
	/// \todo: Check that these are constructed correctly!
	addAttributeToHeader<CqMatrix,float*>("matrixWorldToScreen",
			TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, header, *this);
	addAttributeToHeader<CqMatrix,float*>("matrixWorldToCamera",
			TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, header, *this);
}

void CqTiffDirHandle::fillHeaderPixelLayout(CqTexFileHeader& header) const
{
	// Deal with fields which determine the pixel layout.
	try
	{
		// Deduce image channel information.
		guessChannels(header.findAttribute<CqChannelList>("channels"));
		// Check that channels are interlaced, otherwise we'll be confused.
		TqInt planarConfig = tiffTagValue<uint16>(TIFFTAG_PLANARCONFIG,
				PLANARCONFIG_CONTIG);
		if(planarConfig != PLANARCONFIG_CONTIG)
			throw XqUnknownTiffFormat("non-interlaced channels detected",
					__FILE__, __LINE__);
		// Check that the origin is at the topleft of the image.
		TqInt orientation = tiffTagValue<uint16>(TIFFTAG_ORIENTATION,
				ORIENTATION_TOPLEFT);
		if(orientation != ORIENTATION_TOPLEFT)
			throw XqUnknownTiffFormat("orientation isn't top-left",
					__FILE__, __LINE__);
	}
	catch(XqUnknownTiffFormat& e)
	{
		Aqsis::log() << warning
			<< "Cannot handle desired tiff format efficiently: \"" << e.what() << "\".\n"
			"Switching to generic RGBA handling - this may result in some loss of precision\n";
		// The format is something strange that we don't know how to handle
		// directly... Use the generic RGBA handling built into libtiff...
		EqChannelType chanType = Channel_Unsigned8;
		CqChannelList channels;
		channels.addChannel( SqChannelInfo("r", chanType) );
		channels.addChannel( SqChannelInfo("g", chanType) );
		channels.addChannel( SqChannelInfo("b", chanType) );
		channels.addChannel( SqChannelInfo("a", chanType) );
		header.setAttribute<CqChannelList>("channels", channels);
		// For the moment, throw another error here.
		/// \todo Make the generic RGBA handling work properly.
		throw XqInternal("Cannot handle desired tiff format", __FILE__, __LINE__);
	}
}

EqChannelType CqTiffDirHandle::guessChannelType() const
{
	TqInt bitsPerSample = tiffTagValue<uint16>(TIFFTAG_BITSPERSAMPLE);
	TqInt sampleFormat = tiffTagValue<uint16>(TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	switch(bitsPerSample)
	{
		case 32:
			switch(sampleFormat)
			{
				case SAMPLEFORMAT_IEEEFP:
					return Channel_Float32;
				case SAMPLEFORMAT_INT:
					return Channel_Signed32;
				case SAMPLEFORMAT_UINT:
					return Channel_Unsigned32;
				default:
					Aqsis::log() << warning
						<< "Unknown tiff format for 32 bits per sample: "
						"TIFFTAG_SAMPLEFORMAT = " << sampleFormat
						<< ".  Assuming unsigned int.\n";
					return Channel_Unsigned32;
			}
			break;
		case 16:
			switch(sampleFormat)
			{
				case SAMPLEFORMAT_INT:
					return Channel_Signed16;
				case SAMPLEFORMAT_UINT:
					return Channel_Unsigned16;
				default:
					Aqsis::log() << warning
						<< "Unknown tiff format for 16 bits per sample: "
						"TIFFTAG_SAMPLEFORMAT = " << sampleFormat
						<< ".  Assuming unsigned int.\n";
					return Channel_Unsigned16;
			}
			break;
		case 8:
			switch(sampleFormat)
			{
				case SAMPLEFORMAT_INT:
					return Channel_Signed8;
				case SAMPLEFORMAT_UINT:
					return Channel_Unsigned8;
				default:
					Aqsis::log() << warning
						<< "Unknown tiff format for 8 bits per sample: "
						"TIFFTAG_SAMPLEFORMAT = " << sampleFormat
						<< ".  Assuming unsigned int.\n";
					return Channel_Unsigned8;
			}
			break;
		default:
			// We give up and use the generic 8bpp tiff loading.
			return Channel_TypeUnknown;
	}
}

void CqTiffDirHandle::guessChannels(CqChannelList& channels) const
{
	channels.clear();
	EqChannelType chanType = guessChannelType();
	if(chanType == Channel_TypeUnknown)
		throw XqUnknownTiffFormat("Cannot determine channel type", __FILE__, __LINE__);
	else
	{
		// Determine the channel type held in the tiff
		switch(tiffTagValue<uint16>(TIFFTAG_PHOTOMETRIC))
		{
			case PHOTOMETRIC_MINISBLACK:
				// We have an intensity (y) channel only.
				channels.addChannel(SqChannelInfo("y", chanType));
				break;
			case PHOTOMETRIC_RGB:
				{
					TqInt samplesPerPixel = tiffTagValue<uint16>(TIFFTAG_SAMPLESPERPIXEL);
					if(samplesPerPixel < 3)
						channels.addUnnamedChannels(chanType, samplesPerPixel);
					else
					{
						// add RGB channels
						channels.addChannel(SqChannelInfo("r", chanType));
						channels.addChannel(SqChannelInfo("g", chanType));
						channels.addChannel(SqChannelInfo("b", chanType));
						/// \todo Investigate what to do about TIFFTAG_EXTRASAMPLES
						if(samplesPerPixel == 4)
						{
							// add alpha channel
							channels.addChannel(SqChannelInfo("a", chanType));
						}
						else if(samplesPerPixel == 6)
						{
							// add RGB alpha channels
							channels.addChannel(SqChannelInfo("ra", chanType));
							channels.addChannel(SqChannelInfo("ga", chanType));
							channels.addChannel(SqChannelInfo("ba", chanType));
						}
						else
						{
							// Or not sure what to do here... add some unnamed
							// channels?
							channels.addUnnamedChannels(chanType, samplesPerPixel-3);
						}
					}
				}
				break;
			/// \todo Should also handle the following?
			//case PHOTOMETRIC_LOGL:
			//case PHOTOMETRIC_LOGLUV:
			default:
				throw XqUnknownTiffFormat("Unknown photometric type",
						__FILE__, __LINE__);
		}
	}
}


//------------------------------------------------------------------------------
// CqTiffFileHandle
//------------------------------------------------------------------------------

void safeTiffClose(TIFF* tif)
{
	if(tif)
		TIFFClose(tif);
}

CqTiffFileHandle::CqTiffFileHandle(const std::string& fileName, const char* openMode)
	: m_tiffPtr(TIFFOpen(fileName.c_str(), openMode), safeTiffClose),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		throw XqEnvironment( boost::str(boost::format("Could not open tiff file '%s'")
					% fileName).c_str(), __FILE__, __LINE__);
	}
}

CqTiffFileHandle::CqTiffFileHandle(std::istream& inputStream)
	: m_tiffPtr(TIFFStreamOpen("stream", &inputStream), safeTiffClose),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		throw XqInternal("Could not use input stream for tiff", __FILE__, __LINE__);
	}
}

void CqTiffFileHandle::setDirectory(tdir_t dirIdx)
{
	if(dirIdx != m_currDir)
	{
		if(!TIFFSetDirectory(m_tiffPtr.get(), dirIdx))
			throw XqInternal("Invalid Tiff directory", __FILE__, __LINE__);
		m_currDir = dirIdx;
	}
}


//------------------------------------------------------------------------------
} // namespace Aqsis
