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

const char* tiffFileTypeString = "tiff";

//------------------------------------------------------------------------------
// XqUnknownTiffFormat
//------------------------------------------------------------------------------
/** \brief An exception 
 */
class COMMON_SHARE XqUnknownTiffFormat : public XqInternal
{
	public:
		XqUnknownTiffFormat (const std::string& reason, const std::string& detail,
			const std::string& file, const unsigned int line)
			: XqInternal(reason, detail, file, line)
		{ }

		XqUnknownTiffFormat (const std::string& reason,	const std::string& file,
			const unsigned int line)
			: XqInternal(reason, file, line)
		{ }

		virtual const char* description () const
		{
			return "XqUnknownTiffFormat";
		}

		virtual ~XqUnknownTiffFormat () throw ()
		{ }
};


//------------------------------------------------------------------------------
// Helper functions and data for dealing with tiff compression
//------------------------------------------------------------------------------
typedef std::pair<uint16, const char*> TqComprPair;
static TqComprPair comprTypesInit[] = {
	TqComprPair(COMPRESSION_NONE, "none"),
	TqComprPair(COMPRESSION_LZW, "lzw"),
	TqComprPair(COMPRESSION_JPEG, "jpeg"),
	TqComprPair(COMPRESSION_PACKBITS, "packbits"),
	TqComprPair(COMPRESSION_SGILOG, "log"),
	TqComprPair(COMPRESSION_DEFLATE, "deflate"),
};
/// vector holding the TIFF compression alternatives that we want to deal with.
static const std::vector<TqComprPair> compressionTypes( comprTypesInit,
		comprTypesInit + sizeof(comprTypesInit)/sizeof(TqComprPair));

/// Get the tiff compression type from a string description.
uint16 tiffCompressionTagFromName(const std::string& compressionName)
{
	for(std::vector<TqComprPair>::const_iterator i = compressionTypes.begin();
			i != compressionTypes.end(); ++i)
	{
		if(i->second == compressionName)
			return i->first;
	}
	return COMPRESSION_NONE;
}

/// Get the tiff compression type from a string description.
const char* tiffCompressionNameFromTag(uint16 compressionType)
{
	for(std::vector<TqComprPair>::const_iterator i = compressionTypes.begin();
			i != compressionTypes.end(); ++i)
	{
		if(i->first == compressionType)
			return i->second;
	}
	return "unknown";
}

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

void CqTiffDirHandle::writeHeader(const CqTexFileHeader& header)
{
	writeRequiredAttrs(header);
	writeOptionalAttrs(header);
}

void CqTiffDirHandle::writeRequiredAttrs(const CqTexFileHeader& header)
{
	// Width, height...
	setTiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH, header.width());
	setTiffTagValue<uint32>(TIFFTAG_IMAGELENGTH, header.height());

	// Orientation & planar config should always be fixed.
	setTiffTagValue<uint16>(TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	setTiffTagValue<uint16>(TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

	// Pixel aspect ratio
	// We have no meaningful resolution unit - we're only interested in pixel
	// aspect ratio, so set the resolution unit to none.
	setTiffTagValue<uint16>(TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
	setTiffTagValue<float>(TIFFTAG_XRESOLUTION, 1.0f);
	setTiffTagValue<float>(TIFFTAG_YRESOLUTION,
			header.findAttribute<TqFloat>("pixelAspectRatio"));

	// Compression-related stuff
	writeCompressionAttrs(header);
	// Channel-related stuff
	writeChannelAttrs(header);

	// Write strip size - AFAICT libtiff uses the values of some other fields
	// (compression) to choose a default, so do this last.
	setTiffTagValue<uint32>(TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiffPtr(), 0));
}

void CqTiffDirHandle::writeCompressionAttrs(const CqTexFileHeader& header)
{
	uint16 compression = tiffCompressionTagFromName(
			header.findAttribute<std::string>("compression"));
	setTiffTagValue<uint16>(TIFFTAG_COMPRESSION, compression);
	if(compression != COMPRESSION_NONE
			&& header.channelList().sharedChannelType() != Channel_Float32)
	{
		// Adding a predictor drastically increases the compression ratios for
		// some types of compression.  Apparently it doesn't work well on
		// floating point data (I didn't test this).
		setTiffTagValue<uint16>(TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
	}
	if(compression == COMPRESSION_JPEG)
	{
		setTiffTagValue<int>(TIFFTAG_JPEGQUALITY,
				header.findAttribute<TqInt>("compressionQuality", 85));
	}
}

void CqTiffDirHandle::writeChannelAttrs(const CqTexFileHeader& header)
{
	const CqChannelList& channelList = header.channelList();
	EqChannelType channelType = channelList.sharedChannelType();
	// Assume that the channel type is uniform across the various channels.
	assert(channelType != Channel_TypeUnknown && channelType != Channel_Float16);
	TqInt numChannels = channelList.numChannels();

	setTiffTagValue<uint16>(TIFFTAG_SAMPLESPERPIXEL, numChannels); 
	setTiffTagValue<uint16>(TIFFTAG_BITSPERSAMPLE, 8*bytesPerPixel(channelType));
	if(numChannels == 1)
	{
		// greyscale image
		setTiffTagValue<uint16>(TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		// \todo PHOTOMETRIC_LOGL alternative for floats
	}
	else
	{
		// Multi-channel images.
		//
		// Use PHOTOMETRIC_RGB as the default photometric type.
		setTiffTagValue<uint16>(TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		/// \todo PHOTOMETRIC_LOGLUV alternative for floats
		if(numChannels > 3)
		{
			/// \todo Set TIFFTAG_EXTRASAMPLES appropriately for the a ra, rg, rb channels
		}
	}
	/// \todo: deal with TIFFTAG_SGILOGDATAFMT
	uint16 sampleFormat = 0;
	switch(channelType)
	{
        case Channel_Float32:
			sampleFormat = SAMPLEFORMAT_IEEEFP;
			break;
        case Channel_Signed32:
        case Channel_Signed16:
        case Channel_Signed8:
			sampleFormat = SAMPLEFORMAT_INT;
			break;
        case Channel_Unsigned32:
        case Channel_Unsigned16:
        case Channel_Unsigned8:
			sampleFormat = SAMPLEFORMAT_UINT;
		default:
			break;
    }
	setTiffTagValue<uint16>(TIFFTAG_SAMPLEFORMAT, sampleFormat);
}

/** Convert a type held in the header to the appropriate type understood by
 * libtiff.
 */
template<typename Tattr, typename Ttiff>
static Ttiff attrTypeToTiff(const Tattr& attr)
{
	return Ttiff(attr);
}
// Specialize for std::string -> const char*
template<>
static const char* attrTypeToTiff(const std::string& attr)
{
	return attr.c_str();
}
// Specialize for CqMatrix -> TqFloat*
template<>
static const float* attrTypeToTiff(const CqMatrix& attr)
{
	return attr.pElements();
}

/**
 * Add an attribute with the given tag and name from the header to the given
 * tiff directory.  If the attribute isn't present in the header, silently do
 * nothing.
 */
template<typename Tattr, typename Ttiff>
static void addAttributeToTiff(const char* attributeName, ttag_t tag,
		const CqTexFileHeader& header, CqTiffDirHandle& dirHandle)
{
	const Tattr* headerVal = header.findAttributePtr<Tattr>(attributeName);
	if(headerVal)
	{
		dirHandle.setTiffTagValue<Ttiff>(tag, 
				attrTypeToTiff<Tattr,Ttiff>(*headerVal), false);
	}
}

void CqTiffDirHandle::writeOptionalAttrs(const CqTexFileHeader& header)
{
	// Add various descriptive strings to the header if they exist
	addAttributeToTiff<std::string,const char*>("software",
			TIFFTAG_SOFTWARE, header, *this);
	addAttributeToTiff<std::string,const char*>("hostName",
			TIFFTAG_HOSTCOMPUTER, header, *this);
	addAttributeToTiff<std::string,const char*>("description",
			TIFFTAG_IMAGEDESCRIPTION, header, *this);
	addAttributeToTiff<std::string,const char*>("dateTime",
			TIFFTAG_DATETIME, header, *this);

	// Add some matrix attributes
	/// \todo: Check that these are converted correctly!
	addAttributeToTiff<CqMatrix,const float*>("worldToScreenMatrix",
			TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, header, *this);
	addAttributeToTiff<CqMatrix,const float*>("worldToCameraMatrix",
			TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, header, *this);

	/** \todo Add the following optional attributes:
	 *  - "textureFormat" TIFFTAG_PIXAR_TEXTUREFORMAT,
	 *  - "textureWrapMode" TIFFTAG_PIXAR_WRAPMODES,
	 *  - "fieldOfViewCotan" TIFFTAG_PIXAR_FOVCOT,
	 *  - "??" TIFFTAG_PIXAR_IMAGEFULLWIDTH, TIFFTAG_PIXAR_IMAGEFULLLENGTH,
	 *       TIFFTAG_XPOSITION, TIFFTAG_YPOSITION
	 */
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
			tiffCompressionNameFromTag(tiffTagValue<uint16>(TIFFTAG_COMPRESSION)) );
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
static void addAttributeToHeader(const char* attributeName, ttag_t tag,
		CqTexFileHeader& header, const CqTiffDirHandle& dirHandle)
{
	Ttiff temp;
	if(TIFFGetField(dirHandle.tiffPtr(), tag, &temp))
		header.setAttribute<Tattr>(attributeName, Tattr(temp));
}

void CqTiffDirHandle::fillHeaderOptionalAttrs(CqTexFileHeader& header) const
{
	// Add various descriptive strings to the header if they exist
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
	addAttributeToHeader<CqMatrix,float*>("worldToScreenMatrix",
			TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, header, *this);
	addAttributeToHeader<CqMatrix,float*>("worldToCameraMatrix",
			TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, header, *this);
}

void CqTiffDirHandle::fillHeaderPixelLayout(CqTexFileHeader& header) const
{
	// Deal with fields which determine the pixel layout.
	try
	{
		// Deduce image channel information.
		guessChannels(header.channelList());
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
		CqChannelList channelList;
		channelList.addChannel( SqChannelInfo("r", chanType) );
		channelList.addChannel( SqChannelInfo("g", chanType) );
		channelList.addChannel( SqChannelInfo("b", chanType) );
		channelList.addChannel( SqChannelInfo("a", chanType) );
		header.setAttribute<CqChannelList>("channelList", channelList);
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

void CqTiffDirHandle::guessChannels(CqChannelList& channelList) const
{
	channelList.clear();
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
				channelList.addChannel(SqChannelInfo("y", chanType));
				break;
			case PHOTOMETRIC_RGB:
				{
					TqInt samplesPerPixel = tiffTagValue<uint16>(TIFFTAG_SAMPLESPERPIXEL);
					if(samplesPerPixel < 3)
						channelList.addUnnamedChannels(chanType, samplesPerPixel);
					else
					{
						// add RGB channels
						channelList.addChannel(SqChannelInfo("r", chanType));
						channelList.addChannel(SqChannelInfo("g", chanType));
						channelList.addChannel(SqChannelInfo("b", chanType));
						/// \todo Investigate what to do about TIFFTAG_EXTRASAMPLES
						if(samplesPerPixel == 4)
						{
							// add alpha channel
							channelList.addChannel(SqChannelInfo("a", chanType));
						}
						else if(samplesPerPixel == 6)
						{
							// add RGB alpha channels
							channelList.addChannel(SqChannelInfo("ra", chanType));
							channelList.addChannel(SqChannelInfo("ga", chanType));
							channelList.addChannel(SqChannelInfo("ba", chanType));
						}
						else
						{
							// Or not sure what to do here... add some unnamed
							// channels?
							channelList.addUnnamedChannels(chanType, samplesPerPixel-3);
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
		throw XqInternal( boost::str(boost::format("Could not open tiff file '%s'")
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

CqTiffFileHandle::CqTiffFileHandle(std::ostream& outputStream)
	: m_tiffPtr(TIFFStreamOpen("stream", &outputStream), safeTiffClose),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		throw XqInternal("Could not use output stream for tiff", __FILE__, __LINE__);
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
