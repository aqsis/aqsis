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

#include <sstream>

#include <tiffio.hxx>

#include "aqsismath.h"
#include "matrix.h"
#include "logging.h"

namespace Aqsis
{

/// An exception for internal usage by the tiff handling code.
AQSIS_DECLARE_EXCEPTION(XqUnknownTiffFormat, XqInternal);

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

CqTiffDirHandle::CqTiffDirHandle(const boost::shared_ptr<CqTiffFileHandle>& fileHandle, const TqInt dirIdx)
	: m_fileHandle(fileHandle)
{
	if(dirIdx > 0)
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
	setTiffTagValue<float>(TIFFTAG_YRESOLUTION, header.find<Attr::PixelAspectRatio>());

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
	uint16 compression = tiffCompressionTagFromName( header.find<Attr::Compression>());
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
				header.find<Attr::CompressionQuality>(85));
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
static void addAttributeToTiff(ttag_t tag,
		const CqTexFileHeader& header, CqTiffDirHandle& dirHandle)
{
	const typename Tattr::type* headerVal = header.findPtr<Tattr>();
	if(headerVal)
	{
		dirHandle.setTiffTagValue<Ttiff>(tag, 
				attrTypeToTiff<typename Tattr::type,Ttiff>(*headerVal), false);
	}
}

void CqTiffDirHandle::writeOptionalAttrs(const CqTexFileHeader& header)
{
	// Add various descriptive strings to the header if they exist
	addAttributeToTiff<Attr::Software,const char*>(TIFFTAG_SOFTWARE, header, *this);
	addAttributeToTiff<Attr::HostName,const char*>(TIFFTAG_HOSTCOMPUTER, header, *this);
	addAttributeToTiff<Attr::Description,const char*>(TIFFTAG_IMAGEDESCRIPTION, header, *this);
	addAttributeToTiff<Attr::DateTime,const char*>(TIFFTAG_DATETIME, header, *this);

	// Add some matrix attributes
	addAttributeToTiff<Attr::WorldToScreenMatrix,const float*>(
			TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, header, *this);
	addAttributeToTiff<Attr::WorldToCameraMatrix,const float*>(
			TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, header, *this);

	/** \todo Add the following optional attributes:
	 *  - "textureFormat" TIFFTAG_PIXAR_TEXTUREFORMAT,
	 *  - "textureWrapMode" TIFFTAG_PIXAR_WRAPMODES,
	 *  - "fieldOfViewCotan" TIFFTAG_PIXAR_FOVCOT,
	 */
	const SqImageRegion* displayWindow = header.findPtr<Attr::DisplayWindow>();
	if(displayWindow)
	{
		setTiffTagValue<uint32>(TIFFTAG_PIXAR_IMAGEFULLWIDTH, displayWindow->width);
		setTiffTagValue<uint32>(TIFFTAG_PIXAR_IMAGEFULLLENGTH, displayWindow->height);
		setTiffTagValue<float>(TIFFTAG_XPOSITION, displayWindow->topLeftX);
		setTiffTagValue<float>(TIFFTAG_YPOSITION, displayWindow->topLeftY);
	}
}

void CqTiffDirHandle::fillHeaderRequiredAttrs(CqTexFileHeader& header) const
{
	// Fill header with general metadata which won't affect the details of the
	// pixel memory layout.
	header.set<Attr::Width>(tiffTagValue<uint32>(TIFFTAG_IMAGEWIDTH));
	header.set<Attr::Height>(tiffTagValue<uint32>(TIFFTAG_IMAGELENGTH));
	if(TIFFIsTiled(tiffPtr()))
	{
		header.set<Attr::TileInfo>( SqTileInfo(
					tiffTagValue<uint32>(TIFFTAG_TILEWIDTH),
					tiffTagValue<uint32>(TIFFTAG_TILELENGTH)) );
	}
	// Get the compression type.
	header.set<Attr::Compression>(
			tiffCompressionNameFromTag(tiffTagValue<uint16>(TIFFTAG_COMPRESSION)) );
	// Compute pixel aspect ratio
	TqFloat xRes = 0;
	TqFloat yRes = 0;
	if(TIFFGetField(tiffPtr(), TIFFTAG_XRESOLUTION, &xRes)
			&& TIFFGetField(tiffPtr(), TIFFTAG_YRESOLUTION, &yRes))
	{
		// yRes/xRes should be the correct quantity corresponding to the
		// pixelAspectRatio used in OpenEXR.
		header.set<Attr::PixelAspectRatio>(yRes/xRes);
	}
	else
	{
		header.set<Attr::PixelAspectRatio>(1.0f);
	}
}

namespace {

/// Extract an attribute from dirHandle and add it to header, if present.
template<typename Tattr, typename Ttiff>
void addAttributeToHeader(ttag_t tag, CqTexFileHeader& header,
		const CqTiffDirHandle& dirHandle)
{
	Ttiff temp;
	if(TIFFGetField(dirHandle.tiffPtr(), tag, &temp))
		header.set<Tattr>(typename Tattr::type(temp));
}

/// Add texture wrap modes to the header if they can be found in the TIFF.
void addWrapModesToHeader(CqTexFileHeader& header, const CqTiffDirHandle& dirHandle)
{
	char* wrapModesStr = 0;
	if(TIFFGetField(dirHandle.tiffPtr(), TIFFTAG_PIXAR_WRAPMODES, &wrapModesStr))
	{
		SqWrapModes modes;
		std::istringstream iss(wrapModesStr);
		std::string wrapMode;
		iss >> wrapMode;
		modes.sWrap = wrapModeFromString(wrapMode);
		iss >> wrapMode;
		modes.tWrap = wrapModeFromString(wrapMode);
		header.set<Attr::WrapModes>(modes);
	}
}

} // unnamed namespace

void CqTiffDirHandle::fillHeaderOptionalAttrs(CqTexFileHeader& header) const
{
	// Add various descriptive strings to the header if they exist
	addAttributeToHeader<Attr::Software,char*>(TIFFTAG_SOFTWARE, header, *this);
	addAttributeToHeader<Attr::HostName,char*>(TIFFTAG_HOSTCOMPUTER, header, *this);
	addAttributeToHeader<Attr::Description,char*>(TIFFTAG_IMAGEDESCRIPTION, header, *this);
	addAttributeToHeader<Attr::DateTime,char*>(TIFFTAG_DATETIME, header, *this);
	addAttributeToHeader<Attr::TextureFormat,char*>(TIFFTAG_PIXAR_TEXTUREFORMAT, header, *this);

	// Add texturemap-specific stuff to the header if it exists.
	addWrapModesToHeader(header, *this);

	// Add some matrix attributes
	addAttributeToHeader<Attr::WorldToScreenMatrix,float*>(
			TIFFTAG_PIXAR_MATRIX_WORLDTOSCREEN, header, *this);
	addAttributeToHeader<Attr::WorldToCameraMatrix,float*>(
			TIFFTAG_PIXAR_MATRIX_WORLDTOCAMERA, header, *this);

	// Retrieve tags relevant to the display window
	// The origin of the image is apparently given in resolution units, but
	// here we want to interpret it as the number of pixels from the top left
	// of the image, hence the lfloor.
	uint32 fullWidth = header.width();
	uint32 fullHeight = header.height();
	float xPos = 0;
	float yPos = 0;
	if( TIFFGetField(tiffPtr(), TIFFTAG_PIXAR_IMAGEFULLWIDTH, &fullWidth)
		| TIFFGetField(tiffPtr(), TIFFTAG_PIXAR_IMAGEFULLLENGTH, &fullHeight)
		| TIFFGetField(tiffPtr(), TIFFTAG_XPOSITION, &xPos)
		| TIFFGetField(tiffPtr(), TIFFTAG_YPOSITION, &yPos)
		// bitwise OR used since we don't want shortcut evaluation
		)
	{
		header.set<Attr::DisplayWindow>( SqImageRegion(fullWidth, fullHeight,
					lfloor(xPos), lfloor(yPos)) );
	}
}

void CqTiffDirHandle::fillHeaderPixelLayout(CqTexFileHeader& header) const
{
	header.set<Attr::TiffUseGenericRGBA>(false);
	// Deal with fields which determine the pixel layout.
	try
	{
		// Deduce image channel information.
		guessChannels(header.channelList());
		// Check that channels are interlaced, otherwise we'll be confused.
		TqInt planarConfig = tiffTagValue<uint16>(TIFFTAG_PLANARCONFIG,
				PLANARCONFIG_CONTIG);
		if(planarConfig != PLANARCONFIG_CONTIG)
			AQSIS_THROW(XqUnknownTiffFormat, "non-interlaced channels detected");
		// Check that the origin is at the topleft of the image.
		TqInt orientation = tiffTagValue<uint16>(TIFFTAG_ORIENTATION,
				ORIENTATION_TOPLEFT);
		if(orientation != ORIENTATION_TOPLEFT)
		{
			Aqsis::log() << warning
				<< "TIFF orientation for file \"" << m_fileHandle->fileName()
				<< "\" is not top-left.  This may result in unexpected results\n";
			/// \todo Decide whether to use generic TIFF loading facilities for this.
			//AQSIS_THROW(XqUnknownTiffFormat, "orientation isn't top-left");
		}
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
		header.set<Attr::ChannelList>(channelList);
		header.set<Attr::TiffUseGenericRGBA>(true);
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
		AQSIS_THROW(XqUnknownTiffFormat, "Cannot determine channel type");
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
				AQSIS_THROW(XqUnknownTiffFormat, "Unknown photometric type");
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
	: m_fileName(fileName),
	m_tiffPtr(TIFFOpen(fileName.c_str(), openMode), safeTiffClose),
	m_isInputFile(openMode[0] == 'r'),
	m_currDir(0)
{
	if(!m_tiffPtr)
		AQSIS_THROW(XqInvalidFile, "Could not open tiff file \"" << fileName << "\"");
}

CqTiffFileHandle::CqTiffFileHandle(std::istream& inputStream)
	: m_tiffPtr(TIFFStreamOpen("stream", &inputStream), safeTiffClose),
	m_isInputFile(true),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		AQSIS_THROW(XqInternal, "Could not use input stream for tiff");
	}
}

CqTiffFileHandle::CqTiffFileHandle(std::ostream& outputStream)
	: m_tiffPtr(TIFFStreamOpen("stream", &outputStream), safeTiffClose),
	m_isInputFile(false),
	m_currDir(0)
{
	if(!m_tiffPtr)
	{
		AQSIS_THROW(XqInternal, "Could not use output stream for tiff");
	}
}

void CqTiffFileHandle::setDirectory(tdir_t dirIdx)
{
	if(m_isInputFile && dirIdx != m_currDir)
	{
		if(!TIFFSetDirectory(m_tiffPtr.get(), dirIdx))
		{
			AQSIS_THROW(XqInternal, "Requested tiff directory "
					<< dirIdx << " out of range for file \""
					<< m_fileName << "\"");
		}
		m_currDir = dirIdx;
	}
}


//------------------------------------------------------------------------------
} // namespace Aqsis